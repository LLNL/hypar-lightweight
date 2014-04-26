#include <stdlib.h>
#include <string.h>
#include <basic.h>
#include <arrayfunctions.h>
#include <boundaryconditions.h>
#include <mpivars.h>
#include <hypar.h>

int SourceFunction(double *source,double *u,void *s,void *m,double t)
{
  HyPar           *solver   = (HyPar*)        s;
  MPIVariables    *mpi      = (MPIVariables*) m;
  int             n,d;
  _DECLARE_IERR_;

  /* extract boundary information to check for and implement sponge BC */
  DomainBoundary  *boundary = (DomainBoundary*) solver->boundary;
  int             nb        = solver->nBoundaryZones;

  int     nvars   = solver->nvars;
  int     ghosts  = solver->ghosts;
  int     ndims   = solver->ndims;
  int     *dim    = solver->dim_local;

  int size = 1;
  for (d=0; d<ndims; d++) size *= (dim[d] + 2*ghosts);

  /* initialize to zero */
  _ArraySetValue_(source,size*nvars,0.0);

  /* Apart from other source terms, implement sponge BC as a source */
  for (n = 0; n < nb; n++) {
    if (!strcmp(boundary[n].bctype,_SPONGE_)) {
      IERR BCSpongeSource(&boundary[n],ndims,nvars,ghosts,dim,solver->x,u,source); 
      CHECKERR(ierr);
    }
  }


  return(0);
}
