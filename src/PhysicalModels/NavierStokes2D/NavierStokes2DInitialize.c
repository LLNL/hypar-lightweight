#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <basic.h>
#include <arrayfunctions.h>
#include <boundaryconditions.h>
#include <physicalmodels/navierstokes2d.h>
#include <mpivars.h>
#include <hypar.h>

int    NavierStokes2DParabolicFunction (double*,double*,void*,void*,double);
double NavierStokes2DComputeCFL        (void*,void*,double,double);
int    NavierStokes2DFlux              (double*,double*,int,void*,double);
int    NavierStokes2DUpwindRoe         (double*,double*,double*,double*,double*,double*,int,void*,double);
int    NavierStokes2DUpwindRF          (double*,double*,double*,double*,double*,double*,int,void*,double);
int    NavierStokes2DUpwindLLF         (double*,double*,double*,double*,double*,double*,int,void*,double);
int    NavierStokes2DUpwindSWFS        (double*,double*,double*,double*,double*,double*,int,void*,double);
int    NavierStokes2DRoeAverage        (double*,double*,double*,void*);
int    NavierStokes2DLeftEigenvectors  (double*,double*,void*,int);
int    NavierStokes2DRightEigenvectors (double*,double*,void*,int);

int NavierStokes2DInitialize(void *s,void *m)
{
  HyPar         *solver  = (HyPar*)          s;
  MPIVariables  *mpi     = (MPIVariables*)   m; 
  NavierStokes2D       *physics = (NavierStokes2D*) solver->physics;
  int           ferr     = 0;

  if (solver->nvars != _MODEL_NVARS_) {
    fprintf(stderr,"Error in NavierStokes2DInitialize(): nvars has to be %d.\n",_MODEL_NVARS_);
    return(1);
  }
  if (solver->ndims != _MODEL_NDIMS_) {
    fprintf(stderr,"Error in NavierStokes2DInitialize(): ndims has to be %d.\n",_MODEL_NDIMS_);
    return(1);
  }

  /* default values */
  physics->gamma = 1.4; 
  physics->Pr     = 0.72;
  physics->Re     = -1;
  physics->Minf   = 1.0;
  physics->C1     = 1.458e-6;
  physics->C2     = 110.4;
  strcpy(physics->upw_choice,"roe");

  /* reading physical model specific inputs - all processes */
  if (!mpi->rank) {
    FILE *in;
    printf("Reading physical model inputs from file \"physics.inp\".\n");
    in = fopen("physics.inp","r");
    if (!in) printf("Warning: File \"physics.inp\" not found. Using default values.\n");
    else {
      char word[_MAX_STRING_SIZE_];
      ferr = fscanf(in,"%s",word); if (ferr != 1) return(1);
      if (!strcmp(word, "begin")){
	      while (strcmp(word, "end")){
		      ferr = fscanf(in,"%s",word); if (ferr != 1) return(1);
          if (!strcmp(word, "gamma")) { 
            ferr = fscanf(in,"%lf",&physics->gamma); if (ferr != 1) return(1);
          } else if (!strcmp(word,"upwinding")) {
            ferr = fscanf(in,"%s",physics->upw_choice); if (ferr != 1) return(1);
          } else if (!strcmp(word,"Pr")) {
            ferr = fscanf(in,"%lf",&physics->Pr); if (ferr != 1) return(1);
          } else if (!strcmp(word,"Re")) {
            ferr = fscanf(in,"%lf",&physics->Re); if (ferr != 1) return(1);
          } else if (!strcmp(word,"Minf")) {
            ferr = fscanf(in,"%lf",&physics->Minf); if (ferr != 1) return(1);
          } else if (strcmp(word,"end")) {
            char useless[_MAX_STRING_SIZE_];
            ferr = fscanf(in,"%s",useless); if (ferr != 1) return(ferr);
            printf("Warning: keyword %s in file \"physics.inp\" with value %s not ",word,useless);
            printf("recognized or extraneous. Ignoring.\n");
          }
        }
	    } else {
    	  fprintf(stderr,"Error: Illegal format in file \"physics.inp\".\n");
        return(1);
	    }
    }
    fclose(in);
  }

#ifndef serial
  IERR MPIBroadcast_character(physics->upw_choice,_MAX_STRING_SIZE_,0,&mpi->world); CHECKERR(ierr);
  IERR MPIBroadcast_double(&physics->gamma,1,0,&mpi->world);                        CHECKERR(ierr);
  IERR MPIBroadcast_double(&physics->Pr   ,1,0,&mpi->world);                        CHECKERR(ierr);
  IERR MPIBroadcast_double(&physics->Re   ,1,0,&mpi->world);                        CHECKERR(ierr);
  IERR MPIBroadcast_double(&physics->Minf ,1,0,&mpi->world);                        CHECKERR(ierr);
#endif

  /* Scaling the Reynolds number with the M_inf */
  physics->Re /= physics->Minf;

  /* initializing physical model-specific functions */
  solver->ComputeCFL  = NavierStokes2DComputeCFL;
  solver->FFunction   = NavierStokes2DFlux;
  if      (!strcmp(physics->upw_choice,_ROE_ )) solver->Upwind = NavierStokes2DUpwindRoe;
  else if (!strcmp(physics->upw_choice,_RF_  )) solver->Upwind = NavierStokes2DUpwindRF;
  else if (!strcmp(physics->upw_choice,_LLF_ )) solver->Upwind = NavierStokes2DUpwindLLF;
  else if (!strcmp(physics->upw_choice,_SWFS_)) solver->Upwind = NavierStokes2DUpwindSWFS;
  else {
    fprintf(stderr,"Error in NavierStokes2DInitialize(): %s is not a valid upwinding scheme.\n",
            physics->upw_choice);
    return(1);
  }
  solver->AveragingFunction     = NavierStokes2DRoeAverage;
  solver->GetLeftEigenvectors   = NavierStokes2DLeftEigenvectors;
  solver->GetRightEigenvectors  = NavierStokes2DRightEigenvectors;

  /* set the value of gamma in all the boundary objects */
  int n;
  DomainBoundary  *boundary = (DomainBoundary*) solver->boundary;
  for (n = 0; n < solver->nBoundaryZones; n++)  boundary[n].gamma = physics->gamma;

  /* finally, hijack the main solver's dissipation function pointer
   * to this model's own function, since it's difficult to express 
   * the dissipation terms in the general form                      */
  solver->ParabolicFunction = NavierStokes2DParabolicFunction;

  return(0);
}
