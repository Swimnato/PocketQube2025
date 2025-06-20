#ifndef PIN_DEFINITIONS_H
#define PIN_DEFINITIONS_H

<<<<<<< Updated upstream
#define SD_CS 6
#define CAM_CS 7
#define RAD_CS 3
=======

#define RAD_CS 3

#if OLD_BOARD
#define SD_CS 7
#define CAM_CS 2
#else
#define SD_CS 6
#define CAM_CS 7
#endif
>>>>>>> Stashed changes

#endif