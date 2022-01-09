#include <iostream>
#include <random>
#include "TrafficLight.h"
#include "TrafficObject.h"


/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function.

    // perform vector modification under the lock
    std::unique_lock<std::mutex> uLock(_mutex);
    _condition.wait(uLock, [this] { return !_queue.empty(); }); // pass unique lock to condition variable

    // remove last vector element from queue
    T msg = std::move(_queue.back());
    _queue.pop_back();

    return msg;
}


template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.

    // perform vector modification under the lock
    std::lock_guard<std::mutex> ulock(_mutex);

    // add vector to queue
    std::cout << "   Message" << msg << " will be added to the queue" << std::endl;
    _queue.push_back(std::move(msg));
    _condition.notify_one(); // notify client after pushing new T into deque
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

TrafficLight::~TrafficLight(){}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while(true)
    {
        auto light_phase = _msg_queue.receive();
        if (light_phase == TrafficLightPhase::green) { std::cout << "Green light" << std::endl; return; }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));        
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    threads.emplace_back(std::thread (&TrafficLight::cycleThroughPhases,this));
}


void TrafficLight::cycleThroughPhases()
{
    
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles.
    
    //Infinite loop to cycle through traffic light phases
    while(true)
    {
        auto t_start = std::chrono::high_resolution_clock::now();        
        std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 6000 + 4000));

        if (this->getCurrentPhase() == TrafficLightPhase::red) { this->_currentPhase = TrafficLightPhase::green; }
        else if (this->getCurrentPhase() == TrafficLightPhase::green) { this->_currentPhase = TrafficLightPhase::red; }
        //sends an update method to the message queue using move semantics
        _msg_queue.send(std::move(getCurrentPhase()));
        //Debug print
        std::cout << this->getCurrentPhase() << std::endl;

        auto t_now = std::chrono::high_resolution_clock::now();
        std::chrono::milliseconds elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(t_now - t_start);
        //Sleep to reduce cpu demand
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    
}