#include "PeriodicTasks.h"

// Initialize static variables
std::unordered_map<std::string, std::function<void(void)>> PeriodicTasks::tasks;

/// @brief Calls all periodic tasks
void PeriodicTasks::callTasks() {
	Serial.println("Running tasks...");
	for (const auto& task : tasks) {
		Serial.print("Running task ");
		Serial.println(task.first.c_str());
		task.second();
	}
}

/// @brief Adds a function to the collection of periodic tasks
/// @param name The name to give the task
/// @param callback A pointer to the function callback
/// @return True on success
bool PeriodicTasks::addTask(std::string name, std::function<void(void)> callback) {
	return tasks.emplace(name, callback).second;
}

/// @brief Removes a task from the collection of periodic tasks
/// @param name The name of the task to remove
/// @return True on success
bool PeriodicTasks::removeTask(std::string name) {
	return tasks.erase(name) > 0;
}