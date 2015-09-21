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
    int resources_amount, machines_amount, services_amount, processes_amount, balances_amount;
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
		for(j=0; j<var.resources_amount; j++){
			int resource_used = 0;
			for(k=0; k<var.processes_amount; k++){
				if(sol.process_asignations.at(k) == i){
					resource_used += var.processes.at(k).requirements.at(j);
				}
			}
			var.machines.at(i).resources_used.push_back(resource_used);
		}
	}
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
		if(vec.at(i) == element) return true;
	}
	return false;
}
bool contain_all(vector<int> container, vector<int> searched){
	int i, searched_size = searched.size();
	for(i=0; i < searched_size; i++){
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
	//cout << "fits" << endl;
	int i;
	Machine mach = var.machines.at(machine);
	Process proc = var.processes.at(process);
	/*cout << "var resources_amount " << var.resources_amount << endl;
	cout << "mach capacities " << mach.capacities.size() << endl;
	cout << "mach resources used " << mach.resources_used.size() << endl;
	cout << "var requirements " << proc.requirements.size() << endl;*/
	for(i=0; i < var.resources_amount; i++){
		if((mach.capacities.at(i) - mach.resources_used.at(i)) < proc.requirements.at(i)) return false;
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
	//cout << "service constraint" << endl;
	if(!contains(mach.services, proc.service)){ //service constraint
		//cout << "capacity constraint" << endl;
		if(fits(process, machine)){ //capacity constraint
			//cout << "dependency constraint" << endl;
			if(contain_all(var.neighborhoods.at(mach.neighborhood).services, serv.dependencies)){ //dependencies constraint
				//cout << "spread constraint" << endl;
				if(spread(proc.service, process, machine) >= serv.spread_min){ //spread constraint
					return true;
				}
			}
		}
	}

	return false;
}
bool is_tabu(int process, int machine){
	int i, size = tabu.list.size();
	for(i=0; i < size; i++){
		if(tabu.list.at(i).process == process && tabu.list.at(i).machine == machine) return true;
	}
	return false;
}

void release_resources(int process, int machine){
	int i, size = var.resources_amount;
	//if the process started in that machine, keep transient resources
	if(sol.initial_solution.at(process) == machine){
		for(i=0; i < size; i++){
			if(!var.resources.at(i).transient){
				var.machines.at(machine).resources_used.at(i) -= var.processes.at(process).requirements.at(i);
			}
		}
	}else{
		for(i=0; i < size; i++){
			var.machines.at(machine).resources_used.at(i) -= var.processes.at(process).requirements.at(i);
		}
	}
}
void charge_resources(int process, int machine){
	int i, size = var.resources_amount;
	//if the process started in that machine, keep transient resources
	if(sol.initial_solution.at(process) == machine){
		for(i=0; i < size; i++){
			if(!var.resources.at(i).transient){
				var.machines.at(machine).resources_used.at(i) += var.processes.at(process).requirements.at(i);
			}
		}
	}else{
		for(i=0; i < size; i++){
			var.machines.at(machine).resources_used.at(i) += var.processes.at(process).requirements.at(i);
		}
	}
}
void update_services(int process, int machine){
	int i;
	
	//add missing services
	Process proc = var.processes.at(process);
	Machine mach = var.machines.at(machine);
	Location loc = var.locations.at(mach.location);
	Neighborhood neigh = var.neighborhoods.at(mach.neighborhood);
	if(!contains(mach.services, proc.service)) mach.services.push_back(proc.service);
	if(!contains(loc.services, proc.service)) loc.services.push_back(proc.service);
	if(!contains(neigh.services, proc.service)) neigh.services.push_back(proc.service);
	
	//remove services
	Machine old_mach = var.machines.at(sol.process_asignations.at(process));
	Location old_loc = var.locations.at(old_mach.location);
	Neighborhood old_neigh = var.neighborhoods.at(old_mach.neighborhood);
	remove(old_mach.services, proc.service);
	if(!machines_contain_service(old_loc.services, proc.service))
		remove(old_loc.services, proc.service);
	if(!machines_contain_service(old_neigh.services, proc.service))
		remove(old_neigh.services, proc.service);
}
void print_solution(){
	int i;
	
	ofstream myfile;
	myfile.open("new_solution.txt");
	
	for(i=0; i < var.processes_amount; i++){
		myfile << sol.process_asignations.at(i) << " ";
	}
	
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
		for(i=0; i < var.processes_amount; i++){
			sol.best_process_asignations.at(i) = sol.process_asignations.at(i);
		}
		print_solution();
	}
}
void assign(int process, int machine){
	release_resources(process, sol.process_asignations.at(process));
	charge_resources(process, machine);
	update_services(process, machine);
	
	sol.process_asignations.at(process) = machine;
	update_solution_value();
}

void greedy(){
	int i, init, end, aux, machine;
	unsigned int j;
	vector<int> processes;
	//cout << "filling array" << endl;
	for(i=0; i < var.processes_amount; i++){
		processes.push_back(i);
	}
	//cout << "swaping order to make it more random" << endl;
	for(i=0; i < var.processes_amount/2; i++){
		init = rand() % var.processes_amount;
		end = rand() % var.processes_amount;

		aux = processes.at(init);
		processes.at(init) = processes.at(end);
		processes.at(end) = aux;
	}
	vector<int> possibilities;
	//cout << "filling possibilities" << endl;
	for(i=0; i < var.processes_amount; i++){
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
}
void tabu_search(){
	int process, machine;
	double chain = 1;
	double random = ((double)rand()/RAND_MAX);
	//cout << "Outter random value: " << random << endl;
	while(random < chain){
		do{
			process = rand() % var.processes_amount;
			machine = rand() % var.machines_amount;
			//cout << "process: " << process << ", machine: " << machine << endl;
		}while(is_tabu(process, machine) || !valid_move(process, machine));
		assign(process, machine);
		//choose randomly if chain move
		chain = chain * 0.5;
		random = ((double)rand()/RAND_MAX);
		//cout << "Inner random value: " << random << endl;
	}
}

int main( int argc, char *argv[] ){
	time_t timer1, timer2;
    time(&timer1);
    srand(strtol(argv[10], NULL, 10));

	cout << "Reading files" << endl;
	read_files(argv[4], argv[6]);
	cout << "Greedy" << endl;
	greedy();

	cout << "Tabu Search" << endl;
	while(true){
		//cout << "new iteration" << endl;
		time(&timer2);
		if(difftime(timer2,timer1) > 300)return 0;
		tabu_search();
	}
	return 0;
}
