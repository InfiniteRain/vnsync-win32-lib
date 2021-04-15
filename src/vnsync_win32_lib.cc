#include <node.h>
#include <Windows.h>
#include <iostream>

using std::string;
using std::to_string;

using v8::Isolate;
using v8::FunctionCallbackInfo;
using v8::Value;
using v8::Array;
using v8::Object;
using v8::String;
using v8::Number;
using v8::Local;
using v8::Exception;
using v8::Boolean;

Local<String> generateLocalString(Isolate* isolate, string str) {
    return String::NewFromUtf8(isolate, str.c_str()).ToLocalChecked();
}

void throwException(Isolate* isolate, string message) {
    auto errorString = generateLocalString(isolate, message);
    isolate->ThrowException(Exception::TypeError(errorString));
}

void getOpenedWindows(const FunctionCallbackInfo<Value>& args) {
    auto isolate = args.GetIsolate();
    auto context = isolate->GetCurrentContext();
    auto openedWindows = Array::New(isolate);
    auto count = 0;

    for (
        auto hwnd = GetTopWindow(NULL); 
        hwnd != NULL; 
        hwnd = GetNextWindow(hwnd, GW_HWNDNEXT)
    ) {
        if (!IsWindowVisible(hwnd)) {
            continue;
        }

        auto length = GetWindowTextLengthW(hwnd);

        if (length == 0) {
            continue;
        }

        wchar_t* title = new wchar_t[length + 1];
        GetWindowTextW(hwnd, title, length + 1);

        auto openedWindow = Object::New(isolate);

        auto hwndKeyString = generateLocalString(isolate, "handle");
        auto titleKeyString = generateLocalString(isolate, "title");
        auto hwndValueNumber = Number::New(isolate, (int64_t)hwnd);
        auto titleValueString = String::NewFromTwoByte(isolate, (uint16_t *) title).ToLocalChecked();

        delete[] title;

        openedWindow->Set(context, titleKeyString, titleValueString);
        openedWindow->Set(context, hwndKeyString, hwndValueNumber);

        openedWindows->Set(context, count, openedWindow);
        
        count++;
    }

    args.GetReturnValue().Set(openedWindows);
}

void windowExists(const FunctionCallbackInfo<Value>& args) {
    auto isolate = args.GetIsolate();

    if (args.Length() < 1 || !args[0]->IsNumber()) {
        throwException(isolate, "Argument 1 is expected to be a number.");
        return;
    }

    auto context = isolate->GetCurrentContext();
    HWND hwnd = (HWND)args[0]->IntegerValue(context).FromMaybe(0);

    auto exists = IsWindow(hwnd);

    args.GetReturnValue().Set(Boolean::New(isolate, exists));
}

void showWindow(const FunctionCallbackInfo<Value>& args) {
    auto isolate = args.GetIsolate();

    if (args.Length() < 1 || !args[0]->IsNumber()) {
        throwException(isolate, "Argument 1 is expected to be a number.");
        return;
    }

    auto context = isolate->GetCurrentContext();
    HWND hwnd = (HWND)args[0]->IntegerValue(context).FromMaybe(0);

    if (!IsWindow(hwnd)) {
        throwException(isolate, "Invalid window handle.");
        return;
    }

    ShowWindow(hwnd, 1);
    SetForegroundWindow(hwnd);
}

void getWindowRectangle(const FunctionCallbackInfo<Value>& args) {
    auto isolate = args.GetIsolate();

    if (args.Length() < 1 || !args[0]->IsNumber()) {
        throwException(isolate, "Argument 1 is expected to be a number.");
        return;
    }

    auto context = isolate->GetCurrentContext();
    HWND hwnd = (HWND)args[0]->IntegerValue(context).FromMaybe(0);

    if (!IsWindow(hwnd)) {
        throwException(isolate, "Invalid window handle.");
        return;
    }

    RECT wrect;
    GetWindowRect(hwnd, &wrect);

    auto windowRectangle = Object::New(isolate);

    auto leftKeyString = generateLocalString(isolate, "left");
    auto topKeyString = generateLocalString(isolate, "top");
    auto rightKeyString = generateLocalString(isolate, "right");
    auto bottomKeyString = generateLocalString(isolate, "bottom");
    auto leftValueNumber = Number::New(isolate, wrect.left);
    auto topValueNumber = Number::New(isolate, wrect.top);
    auto rightValueNumber = Number::New(isolate, wrect.right);
    auto bottomValueNumber = Number::New(isolate, wrect.bottom);

    windowRectangle->Set(context, leftKeyString, leftValueNumber);
    windowRectangle->Set(context, topKeyString, topValueNumber);
    windowRectangle->Set(context, rightKeyString, rightValueNumber);
    windowRectangle->Set(context, bottomKeyString, bottomValueNumber);

    args.GetReturnValue().Set(windowRectangle);
}

void getCursorPosition(const FunctionCallbackInfo<Value>& args) {
    auto isolate = args.GetIsolate();

    POINT cpoint;
    GetCursorPos(&cpoint);

    auto context = isolate->GetCurrentContext();
    auto cursorPoint = Object::New(isolate);

    auto xKeyString = generateLocalString(isolate, "x");
    auto yKeyString = generateLocalString(isolate, "y");
    auto xValueNumber = Number::New(isolate, cpoint.x);
    auto yValueNumber = Number::New(isolate, cpoint.y);

    cursorPoint->Set(context, xKeyString, xValueNumber);
    cursorPoint->Set(context, yKeyString, yValueNumber);

    args.GetReturnValue().Set(cursorPoint);
}

void setCursorPosition(const FunctionCallbackInfo<Value>& args) {
    auto isolate = args.GetIsolate();

    for (auto i = 0; i < 2; i++) {
        if (args.Length() >= i + 1 && args[i]->IsNumber()) {
            continue;
        }

        auto message = "Argument " + to_string(i + 1) + " is expected to be a number.";
        throwException(isolate, message);
        return;
    }

    auto context = isolate->GetCurrentContext();
    auto x = args[0]->IntegerValue(context).FromMaybe(0);
    auto y = args[1]->IntegerValue(context).FromMaybe(0);

    SetCursorPos(x, y);
}

void leftClickDown(const FunctionCallbackInfo<Value>& args) {
    POINT cpoint;
    GetCursorPos(&cpoint);

    mouse_event(MOUSEEVENTF_LEFTDOWN, cpoint.x, cpoint.y, 0, 0);
}

void leftClickUp(const FunctionCallbackInfo<Value>& args) {
    POINT cpoint;
    GetCursorPos(&cpoint);

    mouse_event(MOUSEEVENTF_LEFTUP, cpoint.x, cpoint.y, 0, 0);
}

void initialize(const Local<Object> exports) {
    NODE_SET_METHOD(exports, "getOpenedWindows", getOpenedWindows);
    NODE_SET_METHOD(exports, "windowExists", windowExists);
    NODE_SET_METHOD(exports, "showWindow", showWindow);
    NODE_SET_METHOD(exports, "getWindowRectangle", getWindowRectangle);
    NODE_SET_METHOD(exports, "getCursorPosition", getCursorPosition);
    NODE_SET_METHOD(exports, "setCursorPosition", setCursorPosition);
    NODE_SET_METHOD(exports, "leftClickDown", leftClickDown);
    NODE_SET_METHOD(exports, "leftClickUp", leftClickUp);
}

NODE_MODULE(addon, initialize)
