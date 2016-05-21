/*! @file ParabolicFunctionNC1.5Stage.c
    @author Debojyoti Ghosh
    @brief Evaluate the parabolic term using a "1.5"-stage discretization.
*/

#include <stdlib.h>
#include <basic.h>
#include <arrayfunctions.h>
#include <mpivars.h>
#include <hypar.h>

/*! Evaluate the parabolic term using a "1.5"-stage finite-difference spatial discretization: 
    The parabolic term is assumed to be of the form:
    \f{equation}{
      {\bf P}\left({\bf u}\right) = \sum_{d1=0}^{D-1}\sum_{d2=0}^{D-1} \frac {\partial^2 h_{d1,d2}\left(\bf u\right)} {\partial x_{d1} \partial x_{d2}},
    \f}
    where \f$d1\f$ and \f$d2\f$ are spatial dimension indices, and \f$D\f$ is the total number of spatial dimensions (#HyPar::ndims). This term is
    discretized at a grid point as:
    \f{equation}{
      \left.{\bf P}\left({\bf u}\right)\right|_j = \sum_{d1=0}^{D-1} \sum_{d2=0}^{D-1} \frac { \mathcal{D}_{d1}\mathcal{D}_{d2} \left[ {\bf h}_{d1,d2} \right] } {\Delta x_{d1} \Delta x_{d2}},
    \f}
    where \f$\mathcal{D}\f$ denotes the finite-difference approximation to the first derivative.
    + When \f$d = d1 = d2\f$, the operator \f$\mathcal{D}^2_d\left[\cdot\right] = \mathcal{D}_{d}\mathcal{D}_{d}\left[\cdot\right]\f$ is computed in one step as the finite-difference approximation to the  second derivative (Laplacian) \f$\mathcal{L}_d\left[\cdot\right]\f$ using #HyPar::SecondDerivativePar.
    + When \f$d1 \ne d2 \f$, each of the first derivative approximations are \f$\mathcal{D}_{d1}\f$ and \f$\mathcal{D}_{d2}\f$ are computed separately, and thus the 
      cross-derivative is evaluated in two steps using #HyPar::FirstDerivativePar.

    \b Note: this form of the parabolic term \b does \b allow for cross-derivatives (\f$ d1 \ne d2 \f$).

    To use this form of the parabolic term:
    + specify \b "par_space_type" in solver.inp as \b "nonconservative-1.5stage" (#HyPar::spatial_type_par).
    + the physical model must specify \f${\bf h}_{d1,d2}\left({\bf u}\right)\f$ through #HyPar::HFunction.

    \sa ParabolicFunctionNC2Stage()
*/
int ParabolicFunctionNC1_5Stage(
                                  double  *par, /*!< array to hold the computed parabolic term */
                                  double  *u,   /*!< solution */
                                  void    *s,   /*!< Solver object of type #HyPar */
                                  void    *m,   /*!< MPI object of type #MPIVariables */
                                  double  t     /*!< Current simulation time */
                               )
{
  HyPar         *solver = (HyPar*)        s;
  MPIVariables  *mpi    = (MPIVariables*) m;
  double        *Func   = solver->fluxC;
  double        *Deriv1 = solver->Deriv1;
  double        *Deriv2 = solver->Deriv2;
  int           d, d1, d2, v, p, done;
  double        dxinv1, dxinv2;
  _DECLARE_IERR_;

  int     ndims   = solver->ndims;
  int     nvars   = solver->nvars;
  int     ghosts  = solver->ghosts;
  int     *dim    = solver->dim_local;
  double  *dxinv  = solver->dxinv;
  int     size   = solver->npoints_local_wghosts;

  if (!solver->HFunction) return(0); /* zero parabolic terms */
  solver->count_par++;

  int index[ndims];
  _ArraySetValue_(par,size*nvars,0.0);

  for (d1 = 0; d1 < ndims; d1++) {
    for (d2 = 0; d2 < ndims; d2++) {

      /* calculate the diffusion function */
      IERR solver->HFunction(Func,u,d1,d2,solver,t);                      CHECKERR(ierr);
      if (d1 == d2) {
        /* second derivative with respect to the same independent variable */
        IERR solver->SecondDerivativePar(Deriv2,Func,d1,solver,mpi);      CHECKERR(ierr);
      } else {
        /* cross derivative */
        IERR solver->FirstDerivativePar(Deriv1,Func  ,d1, 1,solver,mpi);  CHECKERR(ierr);
        IERR MPIExchangeBoundariesnD(ndims,nvars,dim,ghosts,mpi,Deriv1);  CHECKERR(ierr);
        IERR solver->FirstDerivativePar(Deriv2,Deriv1,d2,-1,solver,mpi);  CHECKERR(ierr);
      }

      /* calculate the final term - second derivative of the diffusion function */
      done = 0; _ArraySetValue_(index,ndims,0);
      while (!done) {
        _ArrayIndex1D_(ndims,dim,index,ghosts,p);
        _GetCoordinate_(d1,index[d1],dim,ghosts,dxinv,dxinv1);
        _GetCoordinate_(d2,index[d2],dim,ghosts,dxinv,dxinv2);
        for (v=0; v<nvars; v++) par[nvars*p+v] += (dxinv1*dxinv2 * Deriv2[nvars*p+v]);
        _ArrayIncrementIndex_(ndims,dim,index,done);
      }

    }
  }

  if (solver->flag_ib) _ArrayBlockMultiply_(par,solver->iblank,size,nvars);
  return(0);
}
