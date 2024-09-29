#pragma once
#ifndef __CONTROLS_HPP__
#define __CONTROLS_HPP__

#include <GLFW/glfw3.h>
#include <functional>
#include <iostream>

#include "fixed_map.hpp"

/*
general structure:
    glfwCallback is called on an input event
    this callback remaps key to action
    this action is remapped to action callback function
    if it exists, it is called

Action's could be created in runtime, but there is no need to
*/

// #include <concepts>

//maybe block one if another chosen?
enum class Device : int{
    KeyboardMouse,
    Gamepad
    //TODO is it all?
};

//input obviously does not know game state, so this is purely key-press sugar
enum class ActionType : int {
    //calls callback every update()
    //used for applying force in movement, shooting
    Continuous, 
    //calls calback once per press+release
    //used for opening menu's
    OneShot 
};

//TODO should it be in here?
enum class Action : int {
    MoveUp, //for wall climbing i guess
    MoveDown, //for wall climbing i guess
    MoveLeft,
    MoveRight,
    Jump,
    Shoot,
    Block,
    Menu,
    Confirm,
    LAST_ACTION,
};

class InputHandler {
public:
    InputHandler();

    // void update() {
    //     updateActionState()
    // }
    //required for gamepads. Keyboards are handled via callback. Also called in update()
    void pollUpdates();
    
    void setup(GLFWwindow* window);
    
    void rebindKey(Action action, int newKey);

    void rebindGamepadButton(Action action, int newButton);

    void setActionCallback(Action action, const std::function<void(Action)>& callback);

private:
    static void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        auto* handler = reinterpret_cast<InputHandler*>(glfwGetWindowUserPointer(window));
        if (handler && (action == GLFW_PRESS || action == GLFW_RELEASE)) {
            handler->attemptTriggerActionForKey(key, action == GLFW_PRESS);
        }
    }

    static void glfwJoystickConnectCallback(int jid, int event) {
        if (event == GLFW_CONNECTED) {
            std::cout << "joystick" << glfwGetJoystickName(jid) << "connected\n";
        }
        if (event == GLFW_DISCONNECTED) {
            //legit? TODO
            std::cout << "joystick" /* << glfwGetJoystickName(jid) << */ " disconnected\n";
        }
    }

    void attemptTriggerActionForKey(int key, bool isPressed);

    void attemptTriggerActionForGamepadButton(int button, bool isPressed);
    
    void updateActionState(Action action, bool isPressed);
    
private:
    //TODO?
    bool hasJoystick() {return joystickId != -1;}
    int joystickId = -1;
    GLFWgamepadstate gState;
    //finite amount of keys / actions. Might change if action become dynamic
    FixedMap<GLFW_KEY_LAST+1, Action> keyActionMap;
    FixedMap<GLFW_GAMEPAD_BUTTON_LAST+1, Action> buttonActionMap;
    FixedMap<Action::LAST_ACTION, std::function<void(Action)>> actionCallbackMap;
    FixedMap<Action::LAST_ACTION, ActionType> actionTypeMap;
    FixedMap<Action::LAST_ACTION, bool> currentActionStates;
    FixedMap<Action::LAST_ACTION, bool> previousActionStates;
};
#endif // __CONTROLS_H__