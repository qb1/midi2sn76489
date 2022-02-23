#pragma once

#define CONCAT(a, b) a ## b
#define CONCAT2(a, b) CONCAT(a, b)

// Up to 8 args on the same lines... Add more if necessary.
#define PRINT_LOG_1(TYPE, ARG1)                                            do { Serial.print(TYPE); Serial.println(ARG1); } while(0)
#define PRINT_LOG_2(TYPE, ARG1, ARG2)                                      do { Serial.print(TYPE); Serial.print(ARG1); Serial.println(ARG2); } while(0)
#define PRINT_LOG_3(TYPE, ARG1, ARG2, ARG3)                                do { Serial.print(TYPE); Serial.print(ARG1); Serial.print(ARG2); Serial.println(ARG3); } while(0)
#define PRINT_LOG_4(TYPE, ARG1, ARG2, ARG3, ARG4)                          do { Serial.print(TYPE); Serial.print(ARG1); Serial.print(ARG2); Serial.print(ARG3); Serial.println(ARG4); } while(0)
#define PRINT_LOG_5(TYPE, ARG1, ARG2, ARG3, ARG4, ARG5)                    do { Serial.print(TYPE); Serial.print(ARG1); Serial.print(ARG2); Serial.print(ARG3); Serial.print(ARG4); Serial.println(ARG5); } while(0)
#define PRINT_LOG_6(TYPE, ARG1, ARG2, ARG3, ARG4, ARG5, ARG6)              do { Serial.print(TYPE); Serial.print(ARG1); Serial.print(ARG2); Serial.print(ARG3); Serial.print(ARG4); Serial.print(ARG5); Serial.println(ARG6); } while(0)
#define PRINT_LOG_7(TYPE, ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7)        do { Serial.print(TYPE); Serial.print(ARG1); Serial.print(ARG2); Serial.print(ARG3); Serial.print(ARG4); Serial.print(ARG5); Serial.print(ARG6); Serial.println(ARG7); } while(0)
#define PRINT_LOG_8(TYPE, ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8)  do { Serial.print(TYPE); Serial.print(ARG1); Serial.print(ARG2); Serial.print(ARG3); Serial.print(ARG4); Serial.print(ARG5); Serial.print(ARG6); Serial.print(ARG7); Serial.println(ARG8); } while(0)

#define ELEVENTH_ARGUMENT(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, ...) a11
#define COUNT_ARGUMENTS(...) ELEVENTH_ARGUMENT(dummy, ## __VA_ARGS__, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define PRINT_LOG_MSG(TYPE, ...) CONCAT2(PRINT_LOG_, COUNT_ARGUMENTS(__VA_ARGS__))(TYPE, __VA_ARGS__)

#define ERROR_MSG(...) PRINT_LOG_MSG("[!] ", __VA_ARGS__);
#define INFO_MSG(...)  PRINT_LOG_MSG("[*] ", __VA_ARGS__);

#ifdef ENABLE_DEBUG_LOGS

#define DEBUG_MSG(...) PRINT_LOG_MSG("[-] ", __VA_ARGS__);

#else

#define DEBUG_MSG(...) do {} while(0)

#endif // ENABLE_LOGS
