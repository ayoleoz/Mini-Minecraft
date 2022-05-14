# Mini-minecraft

This project creates an ***interactive 3D world exploration and alteration*** program in the style of the popular computer game Minecraft.

Features implemented includes: 

**Game Engine Tick Function and Player Physics**, Procedural Terrain, Efficient Terrain Rendering and Chunking, **Multithreaded Terrain Generation**, Cave Systems, Texturing and Texture Animation, **Day and night cycle**, **Inventory System**, Procedural grass color, Water waves, Post-process Camera Overlay, Shadow mapping

### Implementation details

**Game Engine Tick Function and Player Physics**

For player's inputs, I edited entity.h to add more key-pressed buttons and edited key and mouse events in myGL.cpp to update m_inputs every time a key is pressed or a mouse is clicked. By doing so, the game engine can rotate the camera on mouse movement, enable or disable flightMode, move in 3D directions, and add/remove blocks on mouse clicks. In the player.cpp input processing function, I just match the key pressed with the direction that I want to associate the key with. Then I add accelerations to the speed and also detect if the player is in the flightMode. If not, then if the player is in the sky, then a gravity will be applied to the acceleration. In the computePhysics function, I update the player's position based on the acceleration and velocity calculated from the previous function by multiplying the velocity by a certain factor (0.95) and adding the accelerations.

For collision detection, I create another function called collision detection and use ray casting to check if there are any blocks in the rayDirection. I multiply the outDist by a factor of 0.999 to smooth the collision process. Right now instead of updating rayDirection by multiplying it with the minimum distance calculated, I just set it to glm::vec3(0) because of the debugging issue. I will come back and fix it in the next milestone.

For placing and removing blocks, I use ray casting to check if there are any blocks within 3 units of the player. If there are any blocks overlapping with the center of the screen, I then set the block to STONE if the mouse is right-clicked or EMPTY if left-clicked.

For the tick function, I invoked Player::tick from MyGL::tick and calculated dT using QDateTime::currentMSecsSinceEpoch() to make the movements smoother.

**Multithreaded Terrain Generation**

For the multi-threading part, the terrain expansion function checks the surrounding 5x5 terrain zones each frame to see whether there are blocks that have been set up or not. If not, then I get a thread to set blocks up. If the blocks already exist, then I am going to spawn a thread to create VBO Data for them. On the main thread, I send VBO data to the GPU to draw all the blocks and at the same time destroying all the blocks' VBO data for those chunks that are too far away from the player for efficiency. The hard part for me is to understand the concept of Mutex and Threads and also how different parts of the program communicates with each other. I find the professors’ lecture on multi-threading and the sample code provided very helpful.

**Inventory System**

For inventory, I designed another ui page and attach png images for showing purpose. I used radio button and QLCDNumber for selecting different BlockTypes and keeping track of the remaining number. The inventory UI page will show up after pressing I and the users can select the BlockType with their mouse (the mouse is needed to be shown by switching to the inventory window from the main window). The remaining number of the BlockTypes is updated through setNum(BlockType) functions inside the inventory.cpp, and similarly for setting the current BlockType function as well. I think the difficulty for this task is the part where I have to modify the return type of placeBlock and removeBlock functions inside player.cpp, in such a way that I can know which BlockType to be modified inside mygl.cpp. After placing a block, the remaining number will decrease by one, and after breaking it (removing it), the number will increase by one. And also there is something weird with the display of the inventory UI page, as the png images are not shown as expected, but this does not influence the basic functionality of the inventory system.  

**Day and Night cycle**

For the day and night cycle, I build based on the code that was given in the course website, and start from there to change several things. I change the position of the sun over time so that it could rotate around the world as the day and the night cycle switches. I add a new palette for the color of the sun to make it more realistic in the daytime. The reytracing code is used to calculate the value of the colors based on the rays point from the player. The output color is calculated by putting spherical uvs on the quad. The effect of the sun and the "dusky-colored father away from the sun" is done by interpolating and mixing values of the sun and dusk. In the lambert fragment, the direction of the sun light is set in such way that mimics the actual sunlight with Lambert shading.
