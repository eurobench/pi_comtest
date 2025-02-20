function r=computePI_SIN(FILE,OUT,yaml =1)
% R=COMPUTEPI_SIN(FILE,OUT)
%  takes the input file name FILE and the output file OUT as an input
%  outputs the result as YAML
% R=COMPUTEPI_SIN(FILE,OUT,0)
%  outputs the result as CSV

MatrixIN = dlmread (FILE, ',', 1, 0); %open csv file
t=MatrixIN(:,1);
u=MatrixIN(:,3);
y=MatrixIN(:,2); %order inverted in order to match the PRTS data structure
FOUT=fopen(OUT,'w');
r=sinResp(u,y,t);

if (yaml==1) %default
 L=length(fieldnames(r));

 fprintf(FOUT,"type: 'vector'\n")
 fprintf(FOUT,"label: [");
 i=0;
 for [val,key] = r
   fprintf(FOUT,"%s",key)
   i=i+1;
   if(i<L)
   fprintf(FOUT,", ");
   end
 end
 fprintf(FOUT,"]\n"); 
 fprintf(FOUT,"value: ["); 
 i=0;
 for [val,key] = r
   fprintf(FOUT,"%g",val)
   i=i+1;
   if(i<L)
   fprintf(FOUT,", ");
   end
 end 
 fprintf(FOUT,"]\n");  
 disp(['yaml saved: ', OUT])
 fclose(FOUT);

 else % csv output  

for [val,key] = r
fprintf(FOUT,"%s,%g\n",key,val)
end
disp(['csv saved: ', OUT])
fclose(FOUT);
end
