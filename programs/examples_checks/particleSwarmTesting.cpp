/*! @file
 * \brief Testing of evolutionary objects integration
 *
 * Initially just looking at examples in the documentation
 * http://eodev.sourceforge.net/eo/doc/html/t-eo_easy_p_s_o_8cpp-example.html
 */
#include <iostream>

// save diagnostic state
#pragma GCC diagnostic push

// turn off the specific warning.
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
#pragma GCC diagnostic ignored "-Wunused-parameter"

#include <eo>

// turn the warnings back on
#pragma GCC diagnostic pop

//-----------------------------------------------------------------------------
typedef eoMinimizingFitness FitT;
typedef eoRealParticle<FitT> Particle;
//-----------------------------------------------------------------------------

// the objective function
double real_value(const Particle& _particle) {
  double sum = 0;
  for (unsigned i = 0; i < _particle.size() - 1; i++)
    sum += pow(_particle[i], 2);
  return (sum);
}

int main() {
  const unsigned int VEC_SIZE = 2;
  const unsigned int POP_SIZE = 20;
  const unsigned int NEIGHBORHOOD_SIZE = 5;

  // the population:
  eoPop<Particle> pop;

  // Evaluation
  eoEvalFuncPtr<Particle, double, const Particle&> eval(real_value);

  // position init
  eoUniformGenerator<double> uGen(-3, 3);
  eoInitFixedLength<Particle> random(VEC_SIZE, uGen);

  // velocity init
  eoUniformGenerator<double> sGen(-2, 2);
  eoVelocityInitFixedLength<Particle> veloRandom(VEC_SIZE, sGen);

  // local best init
  eoFirstIsBestInit<Particle> localInit;

  // perform position initialization
  pop.append(POP_SIZE, random);

  // topology
  eoLinearTopology<Particle> topology(NEIGHBORHOOD_SIZE);

  // the full initializer
  eoInitializer<Particle> init(eval, veloRandom, localInit, topology, pop);
  init();

  // bounds
  eoRealVectorBounds bnds(VEC_SIZE, -1.5, 1.5);

  // velocity
  eoStandardVelocity<Particle> velocity(topology, 1, 1.6, 2, bnds);

  // flight
  eoStandardFlight<Particle> flight;

  // Terminators
  eoGenContinue<Particle> genCont1(50);
  eoGenContinue<Particle> genCont2(50);

  // PS flight
  eoEasyPSO<Particle> pso1(genCont1, eval, velocity, flight);

  eoEasyPSO<Particle> pso2(init, genCont2, eval, velocity, flight);

  // flight
  try {
    pso1(pop);
    std::cout << "FINAL POPULATION AFTER PSO n°1:" << std::endl;
    for (unsigned int i = 0; i < pop.size(); ++i)
      std::cout << "\t" << pop[i] << " " << pop[i].fitness() << std::endl;

    pso2(pop);
    std::cout << "FINAL POPULATION AFTER PSO n°2:" << std::endl;
    for (unsigned int i = 0; i < pop.size(); ++i)
      std::cout << "\t" << pop[i] << " " << pop[i].fitness() << std::endl;
  } catch (std::exception& e) {
    std::cout << "exception: " << e.what() << std::endl;
    ;
    exit(EXIT_FAILURE);
  }

  return 0;
}
