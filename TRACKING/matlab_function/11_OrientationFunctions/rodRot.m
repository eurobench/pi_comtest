function out = rodRot(vector,angle)
z = [0 0 1];
out = vector * cos(angle) + cross(z,vector) * sin(angle) + z * dot(z,vector) * (1-cos(angle));
end