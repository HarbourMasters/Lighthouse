#ifndef CONTROL_SCHEMES_H
#define CONTROL_SCHEMES_H

enum ControlScheme {
    CONTROL_SCHEME_RETRO = 0,
    CONTROL_SCHEME_MODERN = 1,
    CONTROL_SCHEME_POCKET = 2,
};

#ifdef __cplusplus
void ControlSchemes_Apply(int scheme);

extern "C" {
#endif

int port_rightStickIsMapped(void);

#ifdef __cplusplus
}
#endif

#endif // CONTROL_SCHEMES_H
