#include "FileIO.h"
#include "Logger.h"
#include <fstream>
#include <cstring>
#include <iostream>
#include <cstdint>

void save_image(void* image_data, std::string fname, uint32_t w, uint32_t h, GLFW_mouse m, bool cursor) {
    fname = "out/" + fname + ".PAM";
    ov("Save Image", fname);
    
    std::ofstream file(fname.c_str(), std::ios::out | std::ios::binary);
    file    <<  "P7"                         << "\n"
            <<  "WIDTH "    << w             << "\n"
            <<  "HEIGHT "   << h             << "\n"
            <<  "DEPTH "    << "4"           << "\n"
            <<  "MAXVAL "   << "255"         << "\n"
            <<  "TUPLTYPE " << "RGB_ALPHA"   << "\n"
            <<  "ENDHDR"    << "\n";
    
    if(!cursor) {
        file.write((const char*)image_data, w*h*4);
    } else {
        int maxsize = w*h*4;
        char* buffer = new char[maxsize];
        memcpy(buffer, image_data, maxsize);
        int xoff = int(m.xpos*4+maxsize)%maxsize;
        int yoff = int(m.ypos*w*4+maxsize)%maxsize;
        for(int i = -2*4; i < 2*4; i++) { buffer[(xoff+yoff+i-w*4*2+maxsize)%maxsize] = UINT8_MAX;   }
        for(int i = -2*4; i < 2*4; i++) { buffer[(xoff+yoff+i-w*4*1+maxsize)%maxsize] = UINT8_MAX;   }
        for(int i = -3*4; i < 3*4; i++) { buffer[(xoff+yoff+i-w*4*0+maxsize)%maxsize] = UINT8_MAX;   }
        for(int i = -2*4; i < 2*4; i++) { buffer[(xoff+yoff+i+w*4*1+maxsize)%maxsize] = UINT8_MAX;   }
        for(int i = -2*4; i < 2*4; i++) { buffer[(xoff+yoff+i+w*4*2+maxsize)%maxsize] = UINT8_MAX;   }
        for(int i = -1*4; i < 1*4; i++) { buffer[(xoff+yoff+i-w*4*1+maxsize)%maxsize] = 0;          }
        for(int i = -2*4; i < 2*4; i++) { buffer[(xoff+yoff+i-w*4*0+maxsize)%maxsize] = 0;          }
        for(int i = -1*4; i < 1*4; i++) { buffer[(xoff+yoff+i+w*4*1+maxsize)%maxsize] = 0;          }
        file.write((const char*)buffer, w*h*4);
        delete[] buffer;
    }
    file.close();
}

void save_sound(void* image_data, std::string fname, uint32_t w, uint32_t h, GLFW_mouse m, bool cursor) {
    int maxsize = w*h*4;
    char* buffer = new char[maxsize];
    memcpy(buffer, image_data, maxsize);

    for(int yoff = 0; yoff < h; yoff++) {
        char* line = new char[w*4];
        for(int xoff = 0; xoff < w*4; xoff++) { line[xoff] = buffer[(xoff+yoff-w*4*0+maxsize)%maxsize]; }
        fname = "out/S" + std::to_string(yoff) + ".sound";
        std::ofstream file(fname.c_str(), std::ios::out | std::ios::binary);
        file.write((const char*)line, w*4);
        file.close();
        delete[] line;
    }
    delete[] buffer;
}

void save_fspec(fsmag256 *fsm, std::string fname, uint32_t w, uint32_t h) {
    fname = "out/" + fname + ".PAM";
    ov("Save Image", fname);

    std::ofstream file(fname.c_str(), std::ios::out | std::ios::binary);
    file    <<  "P7"                         << "\n"
            <<  "WIDTH "    << w             << "\n"
            <<  "HEIGHT "   << h             << "\n"
            <<  "DEPTH "    << "4"           << "\n"
            <<  "MAXVAL "   << "255"         << "\n"
            <<  "TUPLTYPE " << "RGB_ALPHA"   << "\n"
            <<  "ENDHDR"    << "\n";

    int     maxsize     = w*h*4;
    char*   buffer      = new char[maxsize];
    memset(buffer, 0, maxsize);

    for(int j = 0; j < 256; j++) {
        int xoff = int(          j   *4+maxsize )%maxsize;
        int mag = int((fsm->fsm[j] / 4096.0f) * 255.0f);
        int magclamp = (mag > 255) ? 255 : ((mag < 0) ? 0 : mag);
        for(int k = 0; k < magclamp; k++) {
            int yoff = int(k*-1*w*4+maxsize-(w*4))%maxsize;
            for(int i = 0; i < 4; i++) {
                buffer[(xoff+yoff+i-w*4*0+maxsize)%maxsize] = UINT8_MAX;
            }
        }
    }

    file.write((const char*)buffer, w*h*4);
    delete[] buffer;
    file.close();
}

void new_fspec256(fspec256 *fs) {
    for(int i = 0; i < 256; i++) {
        fs->rl[i] = 0.0f;
        fs->im[i] = 0.0f;
    }
}