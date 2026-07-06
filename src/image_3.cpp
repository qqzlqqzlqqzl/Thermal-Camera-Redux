#ifndef _IMAGE_3_CPP
#define _IMAGE_3_CPP

                TS( imageMicros = currentTimeMicros(); )

                // Recording is written by the main thread after final overlays and border
                // composition, so the saved video deterministically matches the display.

                TS( threadData.imageMicros += ( currentTimeMicros() - imageMicros ); ) // track relative benchmarks

#endif
