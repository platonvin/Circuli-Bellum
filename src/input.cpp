#include "input.hpp"

InputHandler::InputHandler() {
    //default keys and buttons
    keyActionMap[GLFW_KEY_W] = Action::MoveUp;
    keyActionMap[GLFW_KEY_S] = Action::MoveDown;
    keyActionMap[GLFW_KEY_A] = Action::MoveLeft;
    keyActionMap[GLFW_KEY_D] = Action::MoveRight;
    keyActionMap[GLFW_KEY_SPACE] = Action::Jump;
    keyActionMap[GLFW_KEY_ESCAPE] = Action::Menu;
    // keyActionMap[GLFW_KEY_ENTER] = Action::Confirm;
    keyActionMap[GLFW_KEY_ENTER] = Action::DrawRNDcard;

    mouseActionMap[GLFW_MOUSE_BUTTON_LEFT] = Action::Shoot;
    mouseActionMap[GLFW_MOUSE_BUTTON_RIGHT] = Action::Block;

    buttonActionMap[GLFW_GAMEPAD_BUTTON_DPAD_UP] = Action::MoveUp;
    buttonActionMap[GLFW_GAMEPAD_BUTTON_DPAD_DOWN] = Action::MoveDown;
    buttonActionMap[GLFW_GAMEPAD_BUTTON_DPAD_LEFT] = Action::MoveLeft;
    buttonActionMap[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT] = Action::MoveRight;
    buttonActionMap[GLFW_GAMEPAD_BUTTON_A] = Action::Jump;
    buttonActionMap[GLFW_GAMEPAD_BUTTON_B] = Action::Shoot;
    buttonActionMap[GLFW_GAMEPAD_BUTTON_X] = Action::Block;
    buttonActionMap[GLFW_GAMEPAD_BUTTON_START] = Action::Menu;
    buttonActionMap[GLFW_GAMEPAD_BUTTON_Y] = Action::Confirm;

    actionTypeMap[Action::MoveUp] = ActionType::Continuous;
    actionTypeMap[Action::MoveDown] = ActionType::Continuous;
    actionTypeMap[Action::MoveLeft] = ActionType::Continuous;
    actionTypeMap[Action::MoveRight] = ActionType::Continuous;
    actionTypeMap[Action::Jump] = ActionType::OneShot;
    actionTypeMap[Action::Menu] = ActionType::OneShot;
    actionTypeMap[Action::Confirm] = ActionType::OneShot;
    actionTypeMap[Action::Block] = ActionType::OneShot;
    actionTypeMap[Action::Shoot] = ActionType::Continuous;
    actionTypeMap[Action::DrawRNDcard] = ActionType::OneShot;
}

void InputHandler::rebindKey(Action action, int newKey) {
    keyActionMap[newKey] = action;
    // apl((int)action);
    // apl((int)newKey);
}

void InputHandler::rebindMouseButton(Action action, int newKey) {
    mouseActionMap[newKey] = action;
    // apl((int)action);
    // apl((int)newKey);
}

void InputHandler::updateMousePosition(double xpos, double ypos) {
    mousePosf = dvec2(double(xpos), double(ypos));
    mousePosi = ivec2(int(xpos), int(ypos));
}

void InputHandler::rebindGamepadButton(Action action, int newButton) {
    buttonActionMap[newButton] = action;
    // apl(newButton);
}

void InputHandler::setActionCallback(Action action, const std::function<void(Action)>& callback) {
    actionCallbackMap[action] = callback;
    // apl(int(action));
}


void InputHandler::attemptTriggerActionForKey(int key, bool isPressed) {
    if (keyActionMap.contains(key)) {
        Action action = keyActionMap[key];
        updateActionState(action, isPressed);
    }
}

void InputHandler::attemptTriggerActionForMouseButton(int button, bool isPressed) {
    if (mouseActionMap.contains(button)) {
        Action action = mouseActionMap[button];
        updateActionState(action, isPressed);
    }
}

void InputHandler::attemptTriggerActionForGamepadButton(int button, bool isPressed) {
    if (buttonActionMap.contains(button)) {
        Action action = buttonActionMap[button];
        if (actionCallbackMap[action]) {
            actionCallbackMap[action](action);
        }
    }
}

//required for gamepads. Keyboards are handled via callback
void InputHandler::pollUpdates() {
    for(int act=0; act<to_underlying(Action::LAST_ACTION); act++){
        updateActionState((Action)act, currentActionStates[act]);
    }
    if (glfwGetGamepadState(GLFW_JOYSTICK_1, &gState)) {
        for (int b = 0; b <= GLFW_GAMEPAD_BUTTON_LAST; b++) {
            bool isPressed = gState.buttons[b] == GLFW_PRESS;
            attemptTriggerActionForGamepadButton(b, isPressed);
        }
        // TODO if i ever find a gamepad...
        // input_speed(gState.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER]);
    }
}

void InputHandler::setup(GLFWwindow* window) {
    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(window, glfwKeyCallback);
    glfwSetMouseButtonCallback(window, glfwMouseButtonCallback);
    glfwSetCursorPosCallback(window, glfwCursorPosCallback);
    glfwSetJoystickCallback(glfwJoystickConnectCallback);
    //TODO?
    joystickId = glfwJoystickPresent(GLFW_JOYSTICK_1) ? -1 : GLFW_JOYSTICK_1;
}

void InputHandler::updateActionState(Action action, bool isPressed) {
    bool wasPressed = previousActionStates[action];
    currentActionStates[action] = isPressed;

    ActionType actionType = actionTypeMap[to_underlying(action)];

    //continuous actions
    if (actionType == ActionType::Continuous) {
        if(isPressed){
            //gets called every frame (update)
            if (actionCallbackMap[to_underlying(action)]) actionCallbackMap[to_underlying(action)](action); 
        }
    }

    //one-shot actions
    if (actionType == ActionType::OneShot) {
        if (isPressed && !wasPressed) {
            //button has been just pressed, trigger the callback once
            if (actionCallbackMap[to_underlying(action)]) actionCallbackMap[to_underlying(action)](action);
        }
    }

    // Update the previous state after processing
    previousActionStates[action] = isPressed;
}