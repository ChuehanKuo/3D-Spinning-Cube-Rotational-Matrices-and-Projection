#include <cmath>
#include <iostream>
#include <vector>
#include <string>
#include <thread>   
#include <chrono>   

class CubeRenderer {
public:
    CubeRenderer(int w = 80, int h = 24) //terminal dimension, default: 80*24
        : width(w), height(h),
          zBuffer(width * height, 0), //depth buffer, zero is a point at infinity
          buffer(width * height, ' '), //character buffer, initialized with spaces
          A(0), B(0), C(0), //rotation angles for different axes, beginning value
          cubeWidth(20),
          distanceFromCam(100),//push cube toward positive z
          zoomlevel(30) {} //projection scaling

    void run() {
        //setup the terminal
        clearScreen(); //clear terminal
        hideCursor(); //hide terminal cursor
        
        
            while (true) {
                //reset buffer, ensuring each frame starts clean
                std::fill(buffer.begin(), buffer.end(), ' ');
                std::fill(zBuffer.begin(), zBuffer.end(), 0);
                
                // Render cube
                renderCube();
                
                // Display and rotate
                display();
                rotate();
                
                // Wait
                std::this_thread::sleep_for(std::chrono::milliseconds(16));
            }
    }


/*    
    ~CubeRenderer() { //destructor, run when program
        showCursor();
    }
*/

private:
    // State variables
    int width, height;

    std::vector<float> zBuffer; //hold zbuffer values
    std::vector<char> buffer; //hold character to draw

    float A, B, C;
    float cubeWidth;
    int distanceFromCam;
    float zoomlevel;
    
    // Terminal control
    void clearScreen() { //refresh screen for each frame
        #ifdef _WIN32
            system("cls");
        #else
            system("clear");
        #endif
    }
    
    void hideCursor() { 
        std::cout << "\x1b[?25l"; //ANSI escape code
    }

    /*
    void showCursor() { 

    std::cout << "\x1b[?25h"; 

    }
    */
    
    //skipped cursor reshowing since I close the terminal after running

    
    //math functions, combined 3 matrices for this final result
    float calculateX(float i, float j, float k) {
        return i * cos(C) * cos(B) + j * cos(C) * sin(B) * sin(A)  - j * sin(C) * cos(A) 
        + k * cos(C) * sin(B) * cos(A) + k * sin(C) * sin(A);
    }
    
    float calculateY(float i, float j, float k) {
        return i * sin(C) * cos(B) + j * sin(C) * sin(B) * sin(A) + j * cos(C) * cos(A)
        + k * sin(C) * sin(B) * cos(A) - k * cos(C) * sin(A);
    }
    
    float calculateZ(float i, float j, float k) {
        return i * -(sin(B)) + j * cos(B) * sin(A) + k * cos(B) * cos(A);
    }
    
    //rendering function
    void renderCube() {
        float increment = 0.8; //density of each side
        
        char chars[] = {'@', '#', '%', '.', '=', '^'}; //characters for each side
        
        for (float x = -cubeWidth; x < cubeWidth; x += increment) {
            for (float y = -cubeWidth; y < cubeWidth; y += increment) {
                //front side
                drawPoint(x, y, -cubeWidth, chars[0]);
                //right side
                drawPoint(cubeWidth, y, x, chars[1]);
                //left side
                drawPoint(-cubeWidth, y, -x, chars[2]);
                //back side
                drawPoint(-x, y, cubeWidth, chars[3]);
                //bottom side
                drawPoint(x, -cubeWidth, -y, chars[4]);
                //top side
                drawPoint(x, cubeWidth, y, chars[5]);
            }
        }
    }
    
    void drawPoint(float x, float y, float z, char character) {
        //calculate 3D coordinates
        float xp = calculateX(x, y, z);
        float yp = calculateY(x, y, z);
        float zp = calculateZ(x, y, z) + distanceFromCam;
        
        if (zp <= 0) return; //behind the camera check, if behind, then skip drawing this point
        
        //project to 2D
        float oneoverz = 1 / zp; //one over z, used for zbuffering
        int screenx = static_cast<int>(width / 2 + zoomlevel * oneoverz * xp * 2); // width/2 for centering the projection
        int screeny = static_cast<int>(height / 2 + zoomlevel * oneoverz * yp); // height/2 for centering the projection
        // width/2 and height/2 are necessary because terminal starts at left-top corner

        //check bounds, make sure projection stay within the terminal screen
        if (screenx < 0 || screenx >= width || screeny < 0 || screeny >= height) return;
        
        //converts 2D screen coords to 1D index for buffers, this is the generic formula: index=y×width+x
        int index = screeny * width + screenx; 
        
        //Z-buffer check
        if (oneoverz > zBuffer[index]) { //zBuffer[idx] holds the closest depth
            zBuffer[index] = oneoverz; 
            /*
            if a point goes closer(over) than current stored point, then update to that point
            meaning only showing closest point
            */
            buffer[index] = character; //draw the character of that point
        }
    }
    
    void display() {
        std::string frame;
        //reserve 80*24+24 memory, +height is for the newline character '\n'
        frame.reserve(width * height + height); 
        
        
        //move cursor to top-left, prepare terminal to overwrite last frame
        frame = "\x1b[H";
        
        for (int y = 0; y < height; y++) {
            //&buffer[y * width] gets address of first character
            //append width number of characters for each row
            frame.append(&buffer[y * width], width); 
            
            frame.push_back('\n');
        }
        
        std::cout << frame << std::flush; //flush so prints the whole screen at once
    }
    
    void rotate() { //increase rotation angles, this is where the spinning happens
        A += 0.05;
        B += 0.05;
        C += 0.01;
    }
};

int main() { 

    CubeRenderer cube;
    cube.run();

    return 0;

}