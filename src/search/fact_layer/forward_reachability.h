#ifndef FACT_LAYER_FORWARD_REACHABILITY
#define FACT_LAYER_FORWARD_REACHABILITY

#include "fact_layer_generator.h"
#include "../successor_generators/successor_generator.h"

class ForwardReachability{
    void forward_reachability(const Task &task,
                              FactLayerGenerator &fact_layer_generator,
                              SuccessorGenerator &generator);
};



#endif //FACT_LAYER_FORWARD_REACHABILITY