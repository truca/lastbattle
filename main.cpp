#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <ctime>
using namespace std;

class Resource{
	public:
    bool transient;
    int weight_load_cost;
};
class Machine{
	public:
    int neighborhood, location;
    vector<int> capacities, safety_capacities, resources_used, machine_move_costs, services;
};
class Service{
	public:
    int spread_min, amount_of_dependencies;
    vector<int> dependencies, locations;
};
class Process{
	public:
    int service, process_move_cost;
    vector<int> requirements;
};
class Balance_Cost{
	public:
    int resource_one, resource_two, target, weight;
};
class Location{
	public:
    vector<int> services, machines;
};
class Neighborhood{
	public:
    vector<int> services, machines;
};
class Variables{
	public:
    int process_move_cost_weight, service_move_cost_weight, machine_move_cost_weight;
    int resources_amount, machines_amount, services_amount, processes_amount, balances_amount, tabu_size, max_time;
    const char* output_filename; 
    const char* best_output_filename;
    time_t timer;
    vector<Resource> resources;
    vector<Machine> machines;
    vector<Service> services;
    vector<Process> processes;
    vector<Balance_Cost> balance_costs;
    vector<Location> locations;
    vector<Neighborhood> neighborhoods;
}var;
class Solution{
	public:
    double best_value, value;
    vector<int> best_process_asignations, process_asignations, initial_solution;
}sol;
class Tabu_Element{
	public:
	int process, machine;
};
class Tabu_List{
	public:
    vector<Tabu_Element> list;
}tabu;

bool contains(vector<int> vec, int element);
void create_neighborhoods_and_locations();
void update_solution_value();
void update_resources();

//falta hacer todas las funciones de lectura
void read_resources(ifstream & model){
	int i;
	model >> var.resources_amount;
	cout << "resources amount: " << var.resources_amount << endl;
	for(i=0; i < var.resources_amount; i++){
		Resource resource;
		model >> resource.transient;
		model >> resource.weight_load_cost;
		var.resources.push_back(resource);
	}
}
void read_machines(ifstream & model){
	int i, j, res;
	model >> var.machines_amount;
	cout << "machines amount: " << var.machines_amount << endl;
	for(i=0; i < var.machines_amount; i++){
		Machine machine;
		model >> machine.neighborhood;
		model >> machine.location;
		for(j=0; j < var.resources_amount; j++){
			model >> res;
			machine.capacities.push_back(res);
		}
		for(j=0; j < var.resources_amount; j++){
			model >> res;
			machine.safety_capacities.push_back(res);
		}
		for(j=0; j < var.machines_amount; j++){
			model >> res;
			machine.machine_move_costs.push_back(res);
		}
		var.machines.push_back(machine);
	}
}
void read_services(ifstream & model){
	int i, j, dependencies, dependency;
	model >> var.services_amount;
	cout << "services amount: " << var.services_amount << endl;
	for(i=0; i< var.services_amount; i++){
		Service service;
		model >> service.spread_min;
		model >> dependencies;
		for(j=0; j<dependencies; j++){
			model >> dependency;
			service.dependencies.push_back(dependency);
		}
		var.services.push_back(service);
	}
}
void read_processes(ifstream & model){
	int i, j, requirement;
	model >> var.processes_amount;
	cout << "processes amount: " << var.processes_amount << endl;
	for(i=0; i< var.processes_amount; i++){
		//cout << "index: " << i << endl;
		Process process;
		model >> process.service;
		//cout << "service: " << process.service << endl;
		for(j=0; j<var.resources_amount; j++){
			model >> requirement;
			//cout << "requerimiento: "<< j << ":" << requirement << endl;
			process.requirements.push_back(requirement);
		}
		model >> process.process_move_cost;
		var.processes.push_back(process);
	}
}
void read_balances(ifstream & model){
	int i;
	model >> var.balances_amount;
	cout << "balances amount: " << var.balances_amount << endl;
	for(i=0; i< var.balances_amount; i++){
		Balance_Cost balance_cost;
		model >> balance_cost.resource_one;
		model >> balance_cost.resource_two;
		model >> balance_cost.target;
		model >> balance_cost.weight;

		var.balance_costs.push_back(balance_cost);
	}
}
void read_weights(ifstream & model){
	model >> var.process_move_cost_weight;
	cout << "process_move_cost_weight: " << var.process_move_cost_weight << endl;
	model >> var.service_move_cost_weight;
	cout << "service_move_cost_weight: " << var.service_move_cost_weight << endl;
	model >> var.machine_move_cost_weight;
	cout << "machine_move_cost_weight: " << var.machine_move_cost_weight << endl;
}
void read_initial_solution(ifstream & solution){
	int i, j, k, assignation;
	for(i=0; i<var.processes_amount; i++){
		solution >> assignation;
		//cout << "index: " << i << " , value: " << assignation << endl;
		sol.initial_solution.push_back(assignation);
		sol.process_asignations.push_back(assignation);
		sol.best_process_asignations.push_back(assignation);
		if(!contains(var.machines.at(assignation).services, var.processes.at(i).service)){
			var.machines.at(assignation).services.push_back(var.processes.at(i).service);
		}
	}
	//calculate used resources
	for(i=0; i<var.machines_amount; i++){
		var.machines.at(i).resources_used.resize(var.resources_amount, 0);
		/*for(j=0; j<var.resources_amount; j++){
			var.machines.at(i).resources_used.push_back(0);
		}*/
	}
	update_resources();
	update_solution_value();
}
void read_files(const char* problem_model, const char* initial_solution){
	//cout << problem_model << ", " << initial_solution << endl;
	ifstream model(problem_model), solution(initial_solution);

	//cout << "Starting reading model" << endl;
	read_resources(model);
	read_machines(model);
	read_services(model);
	read_processes(model);
	read_balances(model);
	read_weights(model);
	create_neighborhoods_and_locations();
	//cout << "Ended reading model" << endl;

	cout << "Starting reading initial solution" << endl;
	read_initial_solution(solution);
	cout << "Ended reading initial solution" << endl;
	
	sol.best_value = -1;
	update_solution_value();
	cout << "Initial solution value: " << sol.value << endl;

	model.close();
	solution.close();
	cout << "Ended Reading files"<< endl;
}

void create_neighborhoods_and_locations(){
	int i,j;
	for(i=0; i<var.machines_amount; i++){
		int loc = var.machines.at(i).location, neigh = var.machines.at(i).neighborhood;
		int n_size = var.neighborhoods.size(), l_size = var.locations.size();
		if(neigh > (n_size - 1)){
			var.neighborhoods.resize(neigh + 1);
			var.neighborhoods.at(neigh).machines.push_back(i);
		}else{
			if(!contains(var.neighborhoods.at(neigh).machines, i)) var.neighborhoods.at(neigh).machines.push_back(i);
		}
		if(loc > (l_size - 1)){
			var.locations.resize(loc + 1);
			var.locations.at(loc).machines.push_back(i);
		}else{
			if(!contains(var.locations.at(loc).machines, i)) var.locations.at(loc).machines.push_back(i);
		}
		
		for(j=0; j < var.machines.at(i).services.size(); j++){
			if(!contains(var.neighborhoods.at(neigh).services, var.machines.at(i).services.at(j)))
				var.neighborhoods.at(neigh).services.push_back(var.machines.at(i).services.at(j));
			if(!contains(var.locations.at(loc).services, var.machines.at(i).services.at(j)))
				var.locations.at(loc).services.push_back(var.machines.at(i).services.at(j));	
		}
	}
}

bool contains(vector<int> vec, int element){
	int i, size = vec.size();
	for(i=0; i < size; i++){
		/*cout << "into_contains" << endl;
		cout << "into_contains2" << vec.at(i) << element << endl;*/
		if(vec.at(i) == element) return true;
	}
	return false;
}
bool contain_all(vector<int> container, vector<int> searched){
	//cout << "before size" << endl;
	int i, searched_size = searched.size();
	//cout << "after size: " << searched_size << endl;
	for(i=0; i < searched_size; i++){
		/*cout << "into: " << i << endl;
		cout << "into: " << searched.at(i) << endl;*/
		if(!contains(container, searched.at(i))) return false;
	}
	return true;
}
bool machines_contain_service(vector<int> machines, int service){
	int i;
	for(i=0; i < machines.size(); i++){
		if(contains(var.machines.at(machines.at(i)).services, service)) return true;
	}
	return false;
}
void remove(vector<int> container, int element){
	int i;
	for(i=0; i < container.size(); i++){
		if(container.at(i) == element){
			container.erase(container.begin()+i);
		}
	}
}
int available(int machine, int resource){
	Machine mach = var.machines.at(machine);
	return mach.capacities.at(resource) - mach.resources_used.at(resource);
}
bool fits(int process, int machine){
	int i;
	Process proc = var.processes.at(process);
	for(i=0; i < var.resources_amount; i++){
		if(available(machine, i) < proc.requirements.at(i)) return false;
		/*if(available(machine, i) - proc.requirements.at(i) < 0)
			cout << "NEGATIVE, machine: " << machine << ", resource: " << i << endl;*/
	}
	return true;
}
int spread(int service, int process, int machine){
	unsigned int service_spread, i;
	for(i=0; i < var.locations.size(); i++){
		if(contains(var.locations.at(i).services, service)) service_spread++;
	}
	if(!contains(var.locations.at(var.machines.at(machine).location).services, service)) service_spread++;

	return service_spread;
}
bool valid_move(int process, int machine){
	//cout << "valid_move" << endl;
	Machine mach = var.machines.at(machine);
	Process proc = var.processes.at(process);
	Service serv = var.services.at(proc.service);
	//cout << "service" << endl;
	if(!contains(mach.services, proc.service)){ //service constraint
		//cout << "capacity" << endl;
		if(fits(process, machine)){ //capacity constraint
			//cout << "dependencies" << endl;
			if(contain_all(var.neighborhoods.at(mach.neighborhood).services, serv.dependencies)){ //dependencies constraint
				//cout << "spread" << endl;
				if(spread(proc.service, process, machine) >= serv.spread_min){ //spread constraint
					return true;
				}
			}
		}
	}

	return false;
}
bool is_tabu(int process, int machine){
	int i, size = var.tabu_size;
	for(i=0; i < size; i++){
		if(tabu.list.at(i).process == process && tabu.list.at(i).machine == machine) return true;
	}
	return false;
}
void add_tabu(int process, int machine){
	int i;
	for(i=0; i < (var.tabu_size - 1); i++){
		tabu.list.at(i).process = tabu.list.at(i+1).process;
		tabu.list.at(i).machine = tabu.list.at(i+1).machine;
	}
	tabu.list.at(var.tabu_size - 1).process = process;
	tabu.list.at(var.tabu_size - 1).machine = machine;
}

void update_resources(){
	int i, j, k;
	for(i=0; i < var.machines_amount; i++){
		for(j=0; j < var.resources_amount; j++){
			int resource_used = 0;
			for(k=0; k < var.processes_amount; k++){
				if((var.resources.at(j).transient && sol.initial_solution.at(k) == i) || (sol.process_asignations.at(k) == i)){
					resource_used += var.processes.at(k).requirements.at(j);
				}
			}
			var.machines.at(i).resources_used.at(j) = resource_used;
		}
	}
}
void update_services(){
	int i, j, k;
	//machines
	for(i=0; i < var.machines_amount; i++){
		Machine mach = var.machines.at(i);
		mach.services.clear();
		for(j=0; j < var.processes_amount; j++){
			Process proc = var.processes.at(j);
			if(sol.process_asignations.at(j) == i){
				if(!contains(mach.services, proc.service)){
					mach.services.push_back(proc.service);
				}	
			}
		}
	}
	//locations
	for(i=0; i < var.locations.size(); i++){
		Location loc = var.locations.at(i);
		loc.services.clear();
		for(j=0; j < loc.machines.size(); j++){
			Machine mach = var.machines.at(j);
			for(k=0; k < mach.services.size(); k++){
				if(!contains(loc.services, mach.services.at(k))){
					loc.services.push_back(mach.services.at(k));
				}
			}
		}
	}
	//neighborhoods
	for(i=0; i < var.neighborhoods.size(); i++){
		Neighborhood neigh = var.neighborhoods.at(i);
		neigh.services.clear();
		for(j=0; j < neigh.machines.size(); j++){
			Machine mach = var.machines.at(j);
			for(k=0; k < mach.services.size(); k++){
				if(!contains(neigh.services, mach.services.at(k))){
					neigh.services.push_back(mach.services.at(k));
				}
			}
		}
	}
}
void print_solution(){
	int i;
	
	ofstream myfile;
	myfile.open(var.output_filename);
	
	for(i=0; i < var.processes_amount; i++){
		myfile << sol.process_asignations.at(i) << " ";
	}
	
	myfile.close();
}
void print_best_solution(){
	ofstream myfile;
	myfile.open(var.best_output_filename, ios_base::app);
	time_t timer;
	time(&timer);
	
	myfile << difftime(timer,var.timer) << " " << sol.best_value << endl;
	
	myfile.close();
}
void update_solution_value(){
	int i,j;
	double lc=0, bc=0, pmc=0, smc = 0, mmc = 0;
	Machine mach;
	Balance_Cost balance_cost;
	
	//load costs
	for(i=0; i < var.machines_amount; i++){
		mach = var.machines.at(i);
		for(j=0; j < var.resources_amount; j++){
			if(mach.resources_used.at(j) > mach.safety_capacities.at(j))
				lc += var.resources.at(j).weight_load_cost * (mach.resources_used.at(j) - mach.safety_capacities.at(j));
		}
	}
	//balance costs
	for(i=0; i < var.balances_amount; i++){
		balance_cost = var.balance_costs.at(i);
		for(j=0; j < var.machines_amount; j++){
			mach = var.machines.at(j);
			if(balance_cost.target*available(j, balance_cost.resource_one) > available(j, balance_cost.resource_two))
				bc += balance_cost.weight * balance_cost.target*available(j, balance_cost.resource_one) - available(j, balance_cost.resource_two);
		}
	}
	//process move cost
	for(i=0; i < var.processes_amount; i++){
		if(sol.initial_solution.at(i) != sol.process_asignations.at(i)){
			pmc += var.processes.at(i).process_move_cost;
		}
	}
	pmc = pmc * var.process_move_cost_weight;
	//service move cost
	for(i=0; i<var.services_amount; i++){
		int aux = 0;
		for(j=0; j < var.processes_amount; j++){
			if(var.processes.at(j).service == i && sol.initial_solution.at(j) != sol.process_asignations.at(j))
				aux ++;
		}
		if(aux > smc) smc = aux;
	}
	smc = var.service_move_cost_weight * smc;
	//machine move cost
	for(i=0; i < var.processes_amount; i++){
		if(sol.initial_solution.at(i) != sol.process_asignations.at(i)){
			mmc += var.machines.at(sol.initial_solution.at(i)).machine_move_costs.at(sol.process_asignations.at(i));
		}
	}
	mmc = mmc * var.machine_move_cost_weight;
	
	sol.value = lc + bc + pmc + smc + mmc;
	
	if(sol.best_value > sol.value || sol.best_value == -1){
		int i;
		sol.best_value = sol.value;
		cout << "New best solution: " << sol.best_value << endl;
		/*for(i=0; i < var.processes_amount; i++){
			cout << " " << sol.process_asignations.at(i);
		}
		cout << endl;*/
		for(i=0; i < var.processes_amount; i++){
			sol.best_process_asignations.at(i) = sol.process_asignations.at(i);
		}
		print_solution();
		print_best_solution();
	}
}
void assign(int process, int machine){
	sol.process_asignations.at(process) = machine;
	update_services();
	update_resources();
	update_solution_value();
	int i, j;
	for(i=0; i < var.machines_amount; i++){
		for(j=0; j < var.resources_amount; j++){
			//cout << "machine: " << i << ", resource: " << j << ", used: " << var.machines.at(i).resources_used.at(j) << ", capacity: " << var.machines.at(i).capacities.at(j) << ", available: " << available(i,j) << endl;
		}
	}
}

void greedy(){
	int i, init, end, aux, machine;
	time_t timer;
		
	unsigned int j;
	vector<int> processes;
	//cout << "filling array" << endl;
	for(i=0; i < var.processes_amount; i++){
		processes.push_back(i);
	}
	//cout << "swaping order to make it more random" << endl;
	for(i=0; i < var.processes_amount/2; i++){
		time(&timer);
		if(difftime(timer,var.timer) > var.max_time) return;
		init = rand() % var.processes_amount;
		end = rand() % var.processes_amount;

		aux = processes.at(init);
		processes.at(init) = processes.at(end);
		processes.at(end) = aux;
	}
	
	vector<int> possibilities;
	//cout << "filling possibilities" << endl;
	for(i=0; i < var.processes_amount; i++){
		time(&timer);
		if(difftime(timer,var.timer) > var.max_time) return;
		for(j=0; j < 3; j++){
			machine = rand() % var.machines_amount;
			//cout << "validating move" << endl;
			if(valid_move(i, machine)){
				possibilities.push_back(machine);
			}
		}
		double actual_value = sol.value, new_value;
		int process_assignation = sol.process_asignations.at(i);
		//cout << "selecting the best possibility" << endl;
		for(j=0; j < possibilities.size(); j++){
			time(&timer);
			if(difftime(timer,var.timer) > var.max_time) return;
			if(valid_move(i, possibilities.at(j))){
				assign(i, possibilities.at(j));
				new_value = sol.value;
				if(new_value >= actual_value){
					//undo
					assign(i, process_assignation);
				}else{
					actual_value = new_value;
					process_assignation = possibilities.at(j);
				}
			}
		}
		add_tabu(i, process_assignation);
	}
}
void tabu_search(){
	int process, machine, tries;
	
	tries = 0;
	do{
		tries ++;
		process = rand() % var.processes_amount;
		machine = rand() % var.machines_amount;
		time_t timer;
		time(&timer);
		if(difftime(timer,var.timer) > var.max_time) return;
	}while(((is_tabu(process, machine) || !valid_move(process, machine))) && tries < 5);
	assign(process, machine);	
}

int main( int argc, char *argv[] ){
	cout << '\a';
	time_t timer1, timer2;
    time(&timer1);
    var.timer = timer1;
    var.max_time = atoi(argv[2]);
    srand(strtol(argv[10], NULL, 10));
    
	//cout << "argv 12 value: " << argv[12] << endl;
	var.tabu_size = atoi(argv[12]);
	var.output_filename = argv[8];
	var.best_output_filename = argv[14];
	tabu.list.resize(var.tabu_size);

	//cout << "Reading files" << endl;
	read_files(argv[4], argv[6]);
	
	cout << "Greedy" << endl;
	greedy();
	
	cout << "Tabu Search" << endl;
	while(true){
		//cout << "new iteration" << endl;
		time(&timer2);
		if(difftime(timer2,timer1) > var.max_time)return 0;
		//cout << difftime(timer2,timer1) << "time: " << atoi(argv[2]) << endl;
		tabu_search();
	}
	return 0;
}
