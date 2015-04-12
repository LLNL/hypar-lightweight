/* include PETSc header files */
#include <petscsys.h>
#include <petscvec.h>
#include <petscmat.h>
#include <petscts.h>

/* some definitions */
#define _MAX_STRING_SIZE_ 500
#define _EXPLICIT_  0
#define _IMPLICIT_  1

typedef struct _petsccontext_ {
  /* object containing the solver context */
  void *solver;
  /* object containing the MPI context */
  void *mpi;

  PetscReal shift;
  double    waqt;
  int       tic;

  /* flags for implicit treatment */
  int flag_hyperbolic;    /* whether the hyperbolic term is treated implicitly or explicitly    */
  int flag_hyperbolic_f;  /* whether the hyperbolic f - df is treated implicitly or explicitly  */
  int flag_hyperbolic_df; /* whether the hyperbolic df     is treated implicitly or explicitly  */
  int flag_parabolic;     /* hether the parabolic term is treated implicitly or explicitly      */
  int flag_source;        /* hether the source term is treated implicitly or explicitly         */

  /* flags for Jacobian and preconditioning */
  int flag_jfnk_nopre;    /* use unpreconditioned Jacobian-free Newton-Krylov */
  int flag_jfnk_pre;      /* use preconditioned Jacobian-free Newton-Krylov   */

  /* 
    Flag to indicate if the system being solved for implicit time-integration is linear/nonlinear.
    It's decided on the basis of whether user specifies SNES type as ksponly.
  */
  int flag_is_linear;

} PETScContext;

/* Copy Functions */
int TransferVecToPETSc    (double*,Vec,void*);
int TransferVecFromPETSc  (double*,Vec,void*);
int TransferMatToPETSc    (void*,Mat,void*);

/* Register custom time-integration RK/ARKIMEX method */
int PetscRegisterTIMethods (int);

/* Right and left -hand side functions */
PetscErrorCode PetscRHSFunctionExpl (TS,PetscReal,Vec,Vec,void*);
PetscErrorCode PetscRHSFunctionImpl (TS,PetscReal,Vec,Vec,void*);
PetscErrorCode PetscRHSFunctionIMEX (TS,PetscReal,Vec,Vec,void*);
PetscErrorCode PetscIFunctionIMEX   (TS,PetscReal,Vec,Vec,Vec,void*);

/* Jacobian functions for left-hand side (IMEX) */
PetscErrorCode PetscIJacobianIMEX_JFNK_NoPre        (TS,PetscReal,Vec,Vec,PetscReal,Mat,Mat,void*);
PetscErrorCode PetscIJacobianIMEX_JFNK_Pre          (TS,PetscReal,Vec,Vec,PetscReal,Mat,Mat,void*);
PetscErrorCode PetscIJacobianIMEX_Jac_Pre           (TS,PetscReal,Vec,Vec,PetscReal,Mat,Mat,void*);
PetscErrorCode PetscIJacobianIMEX_JFNK_JacIsPre     (TS,PetscReal,Vec,Vec,PetscReal,Mat,Mat,void*);
PetscErrorCode PetscIJacobianIMEX_Jac_NoPre         (TS,PetscReal,Vec,Vec,PetscReal,Mat,Mat,void*);
PetscErrorCode PetscJacobianFunctionIMEX_JFNK       (Mat,Vec,Vec);             
PetscErrorCode PetscJacobianFunctionIMEX_Linear     (Mat,Vec,Vec);             

/* Jacobian functions for right-hand side (implicit) */
PetscErrorCode PetscRHSJacobianImpl_JFNK_NoPre      (TS,PetscReal,Vec,Mat,Mat,void*);
PetscErrorCode PetscRHSJacobianImpl_JFNK_Pre        (TS,PetscReal,Vec,Mat,Mat,void*);
PetscErrorCode PetscRHSJacobianImpl_Jac_Pre         (TS,PetscReal,Vec,Mat,Mat,void*);
PetscErrorCode PetscRHSJacobianImpl_JFNK_JacIsPre   (TS,PetscReal,Vec,Mat,Mat,void*);
PetscErrorCode PetscRHSJacobianImpl_Jac_NoPre       (TS,PetscReal,Vec,Mat,Mat,void*);
PetscErrorCode PetscJacobianFunctionImpl_JFNK       (Mat,Vec,Vec);             
PetscErrorCode PetscJacobianFunctionImpl_Linear     (Mat,Vec,Vec);             

/* Other functions */
PetscErrorCode PetscPreStage        (TS,PetscReal);
PetscErrorCode PetscPostStage       (TS,PetscReal,PetscInt,Vec*);
PetscErrorCode PetscPreTimeStep     (TS);
PetscErrorCode PetscPostTimeStep    (TS);

