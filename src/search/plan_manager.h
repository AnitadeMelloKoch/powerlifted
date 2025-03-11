#include <string>

class PlanManager {

  static std::string plan_filename;
  static std::string pddl_filename;

public:
  PlanManager();

  static std::string get_plan_filename() {
    return plan_filename;
  }

  static void set_plan_filename(std::string s) {
    plan_filename = s;
  }

  static std::string get_pddl_filename() {
    return pddl_filename;
  }

  static void set_pddl_filename(std::string s){
    pddl_filename = s;
  }

};
