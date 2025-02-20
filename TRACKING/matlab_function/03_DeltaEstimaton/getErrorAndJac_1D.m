function [errors,jacobi] = getErrorAndJac_1D(q1,q2,j,j_alt,delta,constraint)

switch constraint
    case 'proj'
        [errors,jacobi] = getErrorAndJac_proj(delta,q1,q2,j);
    case '1d_corr'
        [errors,jacobi] = getErrorAndJac_icorr(delta,q1,q2,j);
    case 'euler_1d'
        [errors,jacobi] = getErrorAndJac_euler1D(delta,q1,q2,j,j_alt);
    case 'euler_2d'
        [errors,jacobi] = getErrorAndJac_euler2D(delta,q1,q2,j,j_alt);
    otherwise
        error('Wrong constraint');
end
end


%% getErrorAndJac_proj
function [errors,jac] = getErrorAndJac_proj(delta,q1,q2,j)

errors = cost_proj(delta,q1,q2,j);
eps = 1e-8;
errors_eps = cost_proj(delta+eps,q1,q2,j);
jac = (errors_eps-errors)/eps;
end


%% cost_proj
function error = cost_proj(delta,q1,q2,j)
q_e2_e1  = getQuat(delta,[0 0 1]);
q_hat = quaternionMultiply(quaternionInvert(q1),quaternionMultiply(q_e2_e1,q2));

alpha = 2*atan2(q_hat(:,4),q_hat(:,1));
q_proj = getQuat(alpha,[0 0 1]);
q_res = quaternionMultiply(quaternionInvert(q_proj),q_hat);
error = getWeight(q1,q2,j).*2.*acos(q_res(:,1));
end


%% getErrorAndJac_euler1D
function [errors,jac] = getErrorAndJac_euler1D(delta,q1,q2,j,j_alt)

errors = cost_euler1D(delta,q1,q2,j,j_alt);

eps = 1e-8;
errors_eps = cost_euler1D(delta+eps,q1,q2,j,j_alt);
jac = (errors_eps-errors)/eps;
end


%% cost_euler1D
function error = cost_euler1D(delta,q1,q2,j,j_alt)
q_e2_e1  = getQuat(delta,[0 0 1]);

q_rot = getQuat(acos(dot([0, 0, 1], j)), cross([0, 0, 1],j));
q_b1_e1 = quaternionMultiply(q1, q_rot);
q_b2_e2 = quaternionMultiply(q2, q_rot);
q_b2_b1 = quaternionMultiply(quaternionMultiply(quaternionInvert(q_b1_e1),q_e2_e1),q_b2_e2);

angles          = getEulerAngles(q_b2_b1,'zxy',true);
second_angle    = angles(:,2);
third_angle     = angles(:,3);

error = getWeight(q1,q2,j).*sqrt((second_angle).^2 + (third_angle).^2);

end


%% getErrorAndJac_1dcorr
function [error,jac] = getErrorAndJac_icorr(delta,q1,q2,j)

v1 = quaternionRotate(q1,j);
v2 = quaternionRotate(q2,j);

weight = getWeight(q1,q2,j);

error = (local_wrapToPi(atan2(v2(:,2), v2(:,1)) - atan2(v1(:,2), v1(:,1))) + delta).*weight;
jac = weight;
end


%% getErrorAndJac_euler2D
function [error,jac] = getErrorAndJac_euler2D(delta,q1,q2,j1,j2)

% Calculate the error
q_e2_e1  = getQuat(delta,[0 0 1]);
q_b2_s2 = getQuat(acos(dot([0, 1, 0], j2)), cross([0, 1, 0], j2));
q_b1_s1 = getQuat(acos(dot([0, 0, 1], j1)), cross([0, 0, 1], j1));

q_e1_b1 = quaternionInvert(quaternionMultiply(q1, q_b1_s1));
q_e2_b1 = quaternionMultiply(q_e1_b1,q_e2_e1);
q_s2_b1 = quaternionMultiply(q_e2_b1,q2);
q_b2_b1 = quaternionMultiply(q_s2_b1,q_b2_s2);

arcsin_arg = 2*(q_b2_b1(:,2).*q_b2_b1(:,1) + q_b2_b1(:,3) .* q_b2_b1(:,4));
second_angle = asin(clip(arcsin_arg,-1,1));

error = (second_angle); 

% Jacobian
q_b2_e2 = quaternionMultiply(q2,q_b2_s2);

d_q3_ba = zeros(1,4);
d_q3_ba(1) = -0.5*sin(delta/2);
d_q3_ba(4) = 0.5*cos(delta/2);
d_q_b = quaternionMultiply(quaternionMultiply(q_e1_b1,d_q3_ba),q_b2_e2);
d_q = d_q_b;

jac = 2*(d_q(:,2) .* q_b2_b1(:,1) + q_b2_b1(:,2).*d_q(:,1)+d_q(:,3).*q_b2_b1(:,4) + q_b2_b1(:,3) .* d_q(:,4));
jac = (jac./sqrt(1 - arcsin_arg.^2));
end


%% clip
function out = clip(in,min,max)
out = in;
if(in<min)
    out = min;
elseif (in>max)
    out = max;
end
end


%% getWeight
function weight = getWeight(q1,q2,j)

v1 = quaternionRotate(q1,j);
v2 = quaternionRotate(q2,j);
weight = sqrt(v1(:,1).^2 + v1(:,2).^2) .* sqrt(v2(:,1).^2 + v2(:,2).^2);
end

function out = local_wrapToPi(in)
    out = in - 2*pi*floor( (in+pi)/(2*pi) )
end
