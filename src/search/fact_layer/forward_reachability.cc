#include "fact_layer_generator.h"
#include "forward_reachability.h"
#include "../successor_generators/successor_generator.h"
#include <iostream>

using namespace std;

void ForwardReachability::forward_reachability(const Task &task,
                                               FactLayerGenerator &fact_layer_generator,
                                               SuccessorGenerator &generator){
    cout << "Starting forward reachability" << endl;

    const auto action_schemas = task.get_action_schemas();

    auto state = task.initial_state;
    const auto applicable = generator.get_applicable_actions(action_schemas, state);

    DBState extended_state;
    bool extended = true;
    while(extended){
        std::tie(extended_state, extended) = fact_layer_generator.generate_next_fact_layer(applicable,
                                                                                  action_schemas,
                                                                                  state);
        task.dump_state(extended_state);
    }
    
    return;
}

