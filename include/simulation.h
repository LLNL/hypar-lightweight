/*! @file simulation.h
    @brief Base class for simulation and declarations for C functions
    @author Debojyoti Ghosh
*/

#ifndef _SIMULATION_H_
#define _SIMULATION_H_

#include <simulation_object.h>
#ifdef with_petsc
#include <petscinterface.h>
#endif

extern "C" int ReadInputs(void*,int,int);/*!< Read the input parameters */
extern "C" int Initialize(void*,int);/*!< Initialize the solver */
extern "C" int InitialSolution(void*,int);/*!< Read the initial solution */
extern "C" int InitializeBoundaries(void*,int);/*!< Initialize the boundary conditions */
extern "C" int InitializeImmersedBoundaries(void*,int);/*!< Initialize the immersed boundary conditions */
extern "C" int InitializePhysics(void*,int);/*!< Initialize the physics */
extern "C" int InitializeSolvers(void*,int);/*!< Initialize the solvers */
extern "C" int Solve(void*,int, int, int);/*!< Solve the PDE - time-integration */
#ifdef with_petsc
extern "C" int SolvePETSc(void*,int, int, int);  /*!< Solve the PDE using PETSc TS */
#endif
extern "C" int Cleanup(void*,int);/*!< Clean up: deallocate all arrays and objects */

/*! Write errors for each simulation */
extern "C" void SimWriteErrors(void*, int, int, double, double);

/*! \class Simulation
    \brief Base class for a simulation
 *  This is a purely virtual base class describing a simulation.
*/

/*! \brief Base class for a simulation
 *
 * This is a purely virtual base class describing a simulation.
*/
class Simulation
{
  public:

    /*! Default constructor */
    Simulation() { }
    /*! Destructor */
    virtual ~Simulation() { }

    /*! Define function */
    virtual int define(int, int) = 0;

    /*! Read solver inputs */
    virtual int ReadInputs() = 0;

    /*! Initialize the simulations */
    virtual int Initialize() = 0;

    /*! Read initial solution for each simulation */
    virtual int InitialSolution() = 0;

    /*! Initialize the boundary conditions of the simulations */
    virtual int InitializeBoundaries() = 0;

    /*! Initialize the immersed boundary conditions of the simulations */
    virtual int InitializeImmersedBoundaries() = 0;

    /*! Initialize the physics of the simulations */
    virtual int InitializePhysics() = 0;

    /*! Initialize the numerical solvers of the simulations */
    virtual int InitializeSolvers() = 0;

    /*! Run the simulation using native time integrators */
    virtual int Solve() = 0;

    /*! Write simulation errors and wall times to file */
    virtual void WriteErrors(double, double) = 0;

    /*! Function to indicate if object is defined */
    virtual bool isDefined() const = 0;

#ifndef serial
    /*! Duplicate MPI communicators */
    virtual int mpiCommDup() = 0;
#endif

#ifdef with_petsc
    /*! Set flag whether to use PETSc time integration */
    virtual void usePetscTS(PetscBool) = 0;

    /*! Run the simulation using PETSc time integrators */
    virtual int SolvePETSc() = 0;
#endif

  protected:

  private:

};

#endif

