#include "MainAppPhysics.h"

//extern float floorLocation[];

void MainAppPhysics_AdvanceGSIM(float floorZOffset)
{
  (void)floorZOffset;
  //floorLocation[2] = floorZOffset;

  // TODO: move the simulation along
}
/*

// Initialise the Tokamak physics engine. 
// Here's where the interesting stuff starts. 
void InitPhysics(void) 
{ 
  cneGeometry *geom;
  cneV3 inertiaTensor;
  cneV3 boxSize1;
  cneV3 floorSize1;
  cneV3 gravity;
  cneV3 pos;
  f32 mass;
  cneSimulatorSizeInfo sizeInfo;
  int i;
  s32 totalBody;
  cneRigidBody *gCubes[CUBECOUNT]; 
  cneAnimatedBody *gFloor;

  // init stack-local structs
  cneV3_Init(&gravity);
  cneV3_Init(&boxSize1);
  cneV3_Init(&floorSize1);
  cneV3_Init(&pos);
  cneV3_Init(&inertiaTensor);

  // Create and initialise the simulator
  // Tell the simulator how many rigid bodies we have
  sizeInfo.rigidBodiesCount = CUBECOUNT;
  // Tell the simulator how many animated bodies we have
  sizeInfo.animatedBodiesCount = 1;
  // Tell the simulator how many bodies we have in total
  totalBody = sizeInfo.rigidBodiesCount + sizeInfo.animatedBodiesCount;
  sizeInfo.geometriesCount = totalBody;
  // The overlapped pairs count defines how many bodies it is possible to be in collision
  // at a single time. The SDK states this should be calculated as:
  // bodies * (bodies-1) / 2
  // So we'll take its word for it. :-)
  sizeInfo.overlappedPairsCount = totalBody * (totalBody - 1) / 2;
  // We're not using any of these so set them all to zero
  sizeInfo.rigidParticleCount = 0;
  sizeInfo.constraintsCount = 0;
  sizeInfo.terrainNodesStartCount = 0;
  
  // Set the gravity. Try changing this to see the effect on the objects
  cneV3_Set(&gravity, 0.0f, -10.0f, 0.0f);

  // Ready to go, create the simulator object
  gSim = cneSimulator_CreateSimulator(&sizeInfo, 0, &gravity);

  // Create rigid bodies for the cubes
  for (i=0; i<CUBECOUNT; i++)
  { 
    // Create a rigid body
    gCubes[i] = cneSimulator_CreateRigidBody(gSim);

    // Add geometry to the body and set it to be a box of dimensions 1, 1, 1
    geom = cneRigidBody_AddGeometry(gCubes[i]);

    cneV3_Set(&boxSize1, 1.0f, 1.0f, 1.0f);
    //geom->SetBoxSize(boxSize1[0], boxSize1[1], boxSize1[2]);
    cneGeometry_SetBoxSizeV3(geom, &boxSize1);
    // Update the bounding info of the object -- must always call this
    // after changing a body's geometry.
    cneRigidBody_UpdateBoundingInfo(gCubes[i]);

    // Set other properties of the object (mass, position, etc.)
    mass = 1.0f;
    inertiaTensor = cneBoxInertiaTensorV3(&boxSize1, mass);
    cneRigidBody_SetInertiaTensorV3(gCubes[i], &inertiaTensor);
    cneRigidBody_SetMass(gCubes[i], mass);

    // Vary the position so the cubes don't all exactly stack on top of each other
    cneV3_Set(&pos,
      (float)(rand()%10) / 100,
      4.0f + i*2.0f,
      (float)(rand()%10) / 100);

    cneRigidBody_SetPos(gCubes[i], &pos);
  }

  // Create an animated body for the floor
  gFloor = cneSimulator_CreateAnimatedBody(gSim);
  // Add geometry to the floor and set it to be a box with size as defined by the FLOORSIZE constant
  geom = cneAnimatedBody_AddGeometry(gFloor);
  cneV3_Set(&floorSize1, FLOOR_WIDTH, FLOOR_HEIGHT, FLOOR_LENGTH);
  cneGeometry_SetBoxSizeV3(geom, &floorSize1);
  cneAnimatedBody_UpdateBoundingInfo(gFloor);
  // Set the position of the floor within the simulator
  cneV3_Set(&pos, FLOOR_X, FLOOR_Y, FLOOR_Z);
  cneAnimatedBody_SetPos(gFloor, &pos);
  // All done
}


void KillPhysics(void)
{
  if (gSim)
  {
    // Destroy the simulator.
    // Note that this will release all related resources that we've allocated.
    cneSimulator_DestroySimulator(gSim);
    gSim = NULL;
  }
}


// LUA script calls this
void MainApp_AdvanceGSIM(lua_Number floorZOffset)
{
  // move the floor
  floorLocation[2] = (float)floorZOffset;

  // TODO: reposition any errant boxes

  // Not calling this for today... tokamak is still pretty unstable now
  //cneSimulator_Advance(gSim, 1.0f / 60.0f, 1, 0);
}

*/