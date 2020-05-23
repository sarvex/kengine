# [assertHelper](assertHelper.hpp)

Helper macros and functions providing higher-level `assert` functionality.

## Members

If `KENGINE_NDEBUG` is defined, all the following macros have no effect.

### kengine_assert(em, condition)

Checks that `condition` is true. If not, breaks the debugger on the current line and calls `assertHelper::assertFailed(em, __FILE__, __LINE__, "condition")`.

### kengine_assert_with_message(em, condition, message)

Same behavior as `kengine_assert`, except the message passed to `assertHelper::assertFailed` is `message`.

### kengine_assert_failed(em, message)

Same behavior as `kengine_assert_with_message`, but with no condition to check. Always breaks the debugger and calls `assertHelper::assertFailed`.

### assertHelper namespace

#### assertFailed

```cpp
void assertFailed(EntityManager & em, const char * file, int line, const char * message);
```

Creates an ImGui window that indicates `file`, `line`, `message` and the call stack. The window has 3 buttons:

* `Skip`: closes the window
* `Ignore`: closes the window and prevents further asserts for the same line
* `Exit`: exits the program

#### isDebuggerPresent

```cpp
bool isDebuggerPresent();
```

Returns whether a debugger is attached.