// -----------------------------------------------------------------------------
// Copyright 2016 Marco Biasini
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//  
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// -----------------------------------------------------------------------------
#ifndef TYPUS_ASYNC_STATE_MACHINE_HH
#define TYPUS_ASYNC_STATE_MACHINE_HH

#include <atomic>

namespace typus {

/**
 * Helper class for implementing asynchronous state machines
 *
 * The asynchronous state machine distinguishes between two types of states. 
 * Transition states and stationary states. Transition states are used to 
 * transition from one stationary state to another, in case the transition is an 
 * asynchronous operation. Examples of such transitions include establishing a 
 * network connection, or starting an external device. When in a transition 
 * state, further requested state changes are delayed until the ongoing transition 
 * finishes.
 */
template <typename State, typename Derived>
class async_state_machine {
public:

    async_state_machine(State initial): 
        desired_(initial), 
        current_(initial) {
    }

    State desired_state() const {
        return desired_.load(std::memory_order_relaxed);
    }
    
    State current_state() const {
        return current.load(std::memory_order_relaxed);
    }

    void set_desired_state(State desired) {
        desired_.store(desired, std::memory_order_relaxed);
        State current = this->current_state();
        if (current == desired || is_transition_state(current)) {
            return;
        }
        this->as_derived().update_state(current, desired);
    }

protected:
    bool begin_transition(State from_state, State transition_state) {
        return current_.compare_exchange(from_state, transition_state);
    }

    void end_transition(State end_state) {
        current_.store(end_state, std::memory_order_relaxed);
        this->set_desired_state(end_state);
    }

private:
    Derived &as_derived() { return static_cast<Derived&>(&this); }
    const Derived &as_derived() const { return static_cast<Derived&>(&this); }

    std::atomic<State> desired_;
    std::atomic<State> current_;

}

}
    
#endif // TYPUS_ASYNC_STATE_MACHINE
