# Functional Demo

## Task description
This demo consists of 3 tasks:
| task | delay | priority |
|------|-------|----------|
| 1    | 7     | 1        |
| 2    | 5     | 2        |
| 3    | 3     | 3        |
| 4    | N/A   | 4        |

## Manual delay
Any button press emulates busy task by preventing dispatching of new tasks.
This prints "Manual Delay" in the UART for every systick skipped.
This is used to debug whether ticks that do not cause task dispatching get accounted for correctly.
