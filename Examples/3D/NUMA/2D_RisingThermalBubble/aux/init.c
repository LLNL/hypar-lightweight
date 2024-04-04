/*

  This code generates the initial solution for the
  2D rising thermal bubble case for the non-hydrostatic
  unified model of the atmosphere (NUMA).

*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

double raiseto(double x, double a)
{
  return(exp(a*log(x)));
}

int main(){

  int     NI,NJ,NK,ndims;
  char    ip_file_type[50];
  FILE    *in;
  strcpy  (ip_file_type,"ascii");

  printf("Reading file \"solver.inp\"...\n");
  in = fopen("solver.inp","r");
  if (!in) printf ("Error: Input file \"solver.inp\" not found. Default values will be used.\n");
  else {
    char word[500];
    fscanf(in,"%s",word);
    if (!strcmp(word, "begin")){
      while (strcmp(word, "end")){
        fscanf(in,"%s",word);
        if (!strcmp(word, "ndims")) fscanf(in,"%d",&ndims);
        else if (!strcmp(word, "size")) {
          fscanf(in,"%d",&NI);
          fscanf(in,"%d",&NJ);
          fscanf(in,"%d",&NK);
        } else if (!strcmp(word, "ip_file_type")) fscanf(in,"%s",ip_file_type);
      }
    } else printf("Error: Illegal format in solver.inp. Crash and burn!\n");
  }
  fclose(in);

  if (ndims != 3) {
    printf("ndims is not 3 in solver.inp. this code is to generate 3D exact solution\n");
    return(0);
  }
  printf("Grid:\t\t\t%d x %d x %d\n", NI, NJ, NK);

  /* read in zero-altitude pressure and temperature */
  double P0 = 100000.0;
  double T0 = 300.0;
  double g     = 9.8;
  double R     = 287.058;
  double gamma = 1.4;
  printf("Reading file \"physics.inp\"...\n");
  in = fopen("physics.inp","r");
  if (!in) printf ("Error: Input file \"physics.inp\" not found. Default values will be used.\n");
  else {
    char word[500];
    fscanf(in,"%s",word);
    if (!strcmp(word, "begin")){
      while (strcmp(word, "end")){
        fscanf(in,"%s",word);
        if      (!strcmp(word, "Pref" )) fscanf(in,"%lf",&P0);
        else if (!strcmp(word, "Tref" )) fscanf(in,"%lf",&T0);
        else if (!strcmp(word, "gamma")) fscanf(in,"%lf",&gamma);
        else if (!strcmp(word, "g"    )) fscanf(in,"%lf",&g);
        else if (!strcmp(word, "R"    )) fscanf(in,"%lf",&R);
      }
    } else printf("Error: Illegal format in physics.inp. Crash and burn!\n");
  }
  fclose(in);
  double inv_gamma_m1 = 1.0 / (gamma-1.0);
  double Cp = gamma * inv_gamma_m1 * R;

  /* Define the domain */
  double xmin, xmax, ymin, ymax, zmin, zmax;
  xmin    =  0.0;
  xmax    =  1000.0;
  ymin    = -10.0;
  ymax    =  10.0;
  zmin    =  0.0;
  zmax    =  1000.0 ;

  double Lx = xmax - xmin;
  double Ly = ymax - ymin;
  double Lz = zmax - zmin;

  int i,j,k;
  double dx = Lx / ((double)NI-1);
  double dy = Ly / ((double)NJ-1);
  double dz = Lz / ((double)NK-1);

  /* Initial perturbation center */
  double xc = 500;
  double zc = 350;

  /* initial perturbation parameters */
  double tc = 0.5;
  double pi = 4.0*atan(1.0);
  double rc = 250.0;

  double *x, *y, *z, *U;
  FILE *out;

   x   = (double*) calloc (NI        , sizeof(double));
  y   = (double*) calloc (NJ        , sizeof(double));
  z   = (double*) calloc (NK        , sizeof(double));
  U   = (double*) calloc (5*NI*NJ*NK, sizeof(double));

  for (i = 0; i < NI; i++){
    for (j = 0; j < NJ; j++){
      for (k = 0; k < NK; k++){
        x[i] = xmin + i*dx;
        y[j] = ymin + j*dy;
        z[k] = zmin + k*dz;
        int p = i + NI*j + NI*NJ*k;

        /* temperature peturbation */
        double r      = sqrt((x[i]-xc)*(x[i]-xc)+(z[k]-zc)*(z[k]-zc));
        double dtheta = (r>rc ? 0.0 : (0.5*tc*(1.0+cos(pi*r/rc))) );

        double theta      = T0 + dtheta;                                            /* temperature potential */
        double Pexner     = 1.0 - (g/(Cp*T0))*z[k];                                 /* Exner pressure        */
        double rho        = (P0/(R*theta)) * raiseto(Pexner,inv_gamma_m1);          /* density */

        double theta_ref  = T0;                                                     /* reference temperature potential  */
        double Pexner_ref = 1.0 - (g/(Cp*T0))*z[k];                                 /* reference Exner pressure         */
        double rho_ref    = (P0/(R*theta_ref)) * raiseto(Pexner_ref,inv_gamma_m1);  /* reference density                */

        U[5*p+0] = rho - rho_ref;
        U[5*p+1] = 0.0;
        U[5*p+2] = 0.0;
        U[5*p+3] = 0.0;
        U[5*p+4] = (rho*theta) - (rho_ref*theta_ref);
      }
    }
  }

  if (!strcmp(ip_file_type,"ascii")) {
    printf("Writing ASCII initial solution file initial.inp\n");
    out = fopen("initial.inp","w");
    for (i = 0; i < NI; i++)  fprintf(out,"%1.16E ",x[i]);
    fprintf(out,"\n");
    for (j = 0; j < NJ; j++)  fprintf(out,"%1.16E ",y[j]);
    fprintf(out,"\n");
    for (k = 0; k < NK; k++)  fprintf(out,"%1.16E ",z[k]);
    fprintf(out,"\n");
    for (k = 0; k < NK; k++)  {
      for (j = 0; j < NJ; j++)  {
        for (i = 0; i < NI; i++)  {
          int p = i + NI*j + NI*NJ*k;
          fprintf(out,"%1.16E ",U[5*p+0]);
        }
      }
    }
    fprintf(out,"\n");
    for (k = 0; k < NK; k++)  {
      for (j = 0; j < NJ; j++)  {
        for (i = 0; i < NI; i++)  {
          int p = i + NI*j + NI*NJ*k;
          fprintf(out,"%1.16E ",U[5*p+1]);
        }
      }
    }
    fprintf(out,"\n");
    for (k = 0; k < NK; k++)  {
      for (j = 0; j < NJ; j++)  {
        for (i = 0; i < NI; i++)  {
          int p = i + NI*j + NI*NJ*k;
          fprintf(out,"%1.16E ",U[5*p+2]);
        }
      }
    }
    fprintf(out,"\n");
    for (k = 0; k < NK; k++)  {
      for (j = 0; j < NJ; j++)  {
        for (i = 0; i < NI; i++)  {
          int p = i + NI*j + NI*NJ*k;
          fprintf(out,"%1.16E ",U[5*p+3]);
        }
      }
    }
    fprintf(out,"\n");
    for (k = 0; k < NK; k++)  {
      for (j = 0; j < NJ; j++)  {
        for (i = 0; i < NI; i++)  {
          int p = i + NI*j + NI*NJ*k;
          fprintf(out,"%1.16E ",U[5*p+4]);
        }
      }
    }
    fprintf(out,"\n");
    fclose(out);

  } else if ((!strcmp(ip_file_type,"binary")) || (!strcmp(ip_file_type,"bin"))) {

    printf("Writing binary initial solution file initial.inp\n");
    out = fopen("initial.inp","wb");
    fwrite(x,sizeof(double),NI,out);
    fwrite(y,sizeof(double),NJ,out);
    fwrite(z,sizeof(double),NK,out);
    fwrite(U,sizeof(double),5*NI*NJ*NK,out);
    fclose(out);

  }

  free(x);
  free(y);
  free(z);
  free(U);

  return(0);
}
