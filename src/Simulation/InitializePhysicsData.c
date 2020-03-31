/*! @file InitializePhysicsData.c
    @author Debojyoti Ghosh
    @brief Read in any arrays/data that physics model wants
*/

#include <stdio.h>
#include <simulation_object.h>

/*! For each simulation object, call the physics-specific function to
    read in any physics data that is not a part of the solution vector.
*/
int InitializePhysicsData(void  *s,   /*!< Simulation object of type #SimulationObject */
                          int   idx,  /*!< Index of this simulation object */
                          int   nsims /*!< Total number of simuations */
                         )
{
  SimulationObject *sim     = (SimulationObject*) s;
  HyPar            *solver  = &(sim->solver);
  MPIVariables     *mpi     = &(sim->mpi);

  if (solver->PhysicsInput) {
    if (!mpi->rank) {
      printf("Domain %d: Initializing physics data for model %s\n",
              idx, solver->model);
    }
    int ierr = solver->PhysicsInput(solver, mpi, idx, nsims);
    if (ierr) {
      fprintf(stderr, "Error in InitializePhysicsData():\n");
      fprintf(stderr, "  solver->PhysicsInput() returned error %d on rank %d\n", 
              ierr, mpi->rank);
      return ierr;
    }
  }

  return 0;
}

