/*
This is free and unencumbered software released into the public domain under The Unlicense.
You have complete freedom to do anything you want with the software, for any purpose.
Please refer to <http://unlicense.org/>
*/

#include <queue>
#include <vector>
#include <functional>
#include <string>
#include <assert.h>

extern "C"
{
  #include "TestUtils.h"
  #include "NateMash.h"
}

char * durpMetronome = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
"<COLLADA xmlns=\"http://www.collada.org/2005/11/COLLADASchema\" version=\"1.4.1\">"
"  <asset>"
"    <contributor>"
"      <author>Blender User</author>"
"      <authoring_tool>Blender 2.71.0 commit date:2014-06-25, commit time:18:36, hash:9337574</authoring_tool>"
"    </contributor>"
"    <created>2015-05-20T23:14:07</created>"
"    <modified>2015-05-20T23:14:07</modified>"
"    <unit name=\"meter\" meter=\"1\"/>"
"    <up_axis>Z_UP</up_axis>"
"  </asset>"
"  <library_cameras>"
"    <camera id=\"Camera-camera\" name=\"Camera\">"
"      <optics>"
"        <technique_common>"
"          <perspective>"
"            <xfov sid=\"xfov\">49.13434</xfov>"
"            <aspect_ratio>1.777778</aspect_ratio>"
"            <znear sid=\"znear\">0.1</znear>"
"            <zfar sid=\"zfar\">100</zfar>"
"          </perspective>"
"        </technique_common>"
"      </optics>"
"      <extra>"
"        <technique profile=\"blender\">"
"          <YF_dofdist>0</YF_dofdist>"
"          <shiftx>0</shiftx>"
"          <shifty>0</shifty>"
"        </technique>"
"      </extra>"
"    </camera>"
"  </library_cameras>"
"  <library_lights>"
"    <light id=\"Lamp-light\" name=\"Lamp\">"
"      <technique_common>"
"        <point>"
"          <color sid=\"color\">1 1 1</color>"
"          <constant_attenuation>1</constant_attenuation>"
"          <linear_attenuation>0</linear_attenuation>"
"          <quadratic_attenuation>0.00111109</quadratic_attenuation>"
"        </point>"
"      </technique_common>"
"      <extra>"
"        <technique profile=\"blender\">"
"          <adapt_thresh>0.000999987</adapt_thresh>"
"          <area_shape>1</area_shape>"
"          <area_size>0.1</area_size>"
"          <area_sizey>0.1</area_sizey>"
"          <area_sizez>1</area_sizez>"
"          <atm_distance_factor>1</atm_distance_factor>"
"          <atm_extinction_factor>1</atm_extinction_factor>"
"          <atm_turbidity>2</atm_turbidity>"
"          <att1>0</att1>"
"          <att2>1</att2>"
"          <backscattered_light>1</backscattered_light>"
"          <bias>1</bias>"
"          <blue>1</blue>"
"          <buffers>1</buffers>"
"          <bufflag>0</bufflag>"
"          <bufsize>2880</bufsize>"
"          <buftype>2</buftype>"
"          <clipend>30.002</clipend>"
"          <clipsta>1.000799</clipsta>"
"          <compressthresh>0.04999995</compressthresh>"
"          <dist sid=\"blender_dist\">29.99998</dist>"
"          <energy sid=\"blender_energy\">1</energy>"
"          <falloff_type>2</falloff_type>"
"          <filtertype>0</filtertype>"
"          <flag>0</flag>"
"          <gamma sid=\"blender_gamma\">1</gamma>"
"          <green>1</green>"
"          <halo_intensity sid=\"blnder_halo_intensity\">1</halo_intensity>"
"          <horizon_brightness>1</horizon_brightness>"
"          <mode>8192</mode>"
"          <ray_samp>1</ray_samp>"
"          <ray_samp_method>1</ray_samp_method>"
"          <ray_samp_type>0</ray_samp_type>"
"          <ray_sampy>1</ray_sampy>"
"          <ray_sampz>1</ray_sampz>"
"          <red>1</red>"
"          <samp>3</samp>"
"          <shadhalostep>0</shadhalostep>"
"          <shadow_b sid=\"blender_shadow_b\">0</shadow_b>"
"          <shadow_g sid=\"blender_shadow_g\">0</shadow_g>"
"          <shadow_r sid=\"blender_shadow_r\">0</shadow_r>"
"          <sky_colorspace>0</sky_colorspace>"
"          <sky_exposure>1</sky_exposure>"
"          <skyblendfac>1</skyblendfac>"
"          <skyblendtype>1</skyblendtype>"
"          <soft>3</soft>"
"          <spotblend>0.15</spotblend>"
"          <spotsize>75</spotsize>"
"          <spread>1</spread>"
"          <sun_brightness>1</sun_brightness>"
"          <sun_effect_type>0</sun_effect_type>"
"          <sun_intensity>1</sun_intensity>"
"          <sun_size>1</sun_size>"
"          <type>0</type>"
"        </technique>"
"      </extra>"
"    </light>"
"  </library_lights>"
"  <library_images/>"
"  <library_effects>"
"    <effect id=\"Material-effect\">"
"      <profile_COMMON>"
"        <technique sid=\"common\">"
"          <phong>"
"            <emission>"
"              <color sid=\"emission\">0 0 0 1</color>"
"            </emission>"
"            <ambient>"
"              <color sid=\"ambient\">0 0 0 1</color>"
"            </ambient>"
"            <diffuse>"
"              <color sid=\"diffuse\">0.64 0.64 0.64 1</color>"
"            </diffuse>"
"            <specular>"
"              <color sid=\"specular\">0.5 0.5 0.5 1</color>"
"            </specular>"
"            <shininess>"
"              <float sid=\"shininess\">50</float>"
"            </shininess>"
"            <index_of_refraction>"
"              <float sid=\"index_of_refraction\">1</float>"
"            </index_of_refraction>"
"          </phong>"
"        </technique>"
"      </profile_COMMON>"
"    </effect>"
"  </library_effects>"
"  <library_materials>"
"    <material id=\"Material-material\" name=\"Material\">"
"      <instance_effect url=\"#Material-effect\"/>"
"    </material>"
"  </library_materials>"
"  <library_geometries>"
"    <geometry id=\"Cube-mesh\" name=\"Cube\">"
"      <mesh>"
"        <source id=\"Cube-mesh-positions\">"
"          <float_array id=\"Cube-mesh-positions-array\" count=\"24\">2.607685 3.291105 -0.2332705 1 -0.2133394 -0.2332705 -1 -0.2133392 -0.2332705 -0.9999997 0.6128335 -0.2332705 0.1993688 0.1993685 3.980571 0.1993686 -0.1993691 3.980571 -0.199369 -0.1993689 3.980571 -0.1993689 0.1993687 3.980571</float_array>"
"          <technique_common>"
"            <accessor source=\"#Cube-mesh-positions-array\" count=\"8\" stride=\"3\">"
"              <param name=\"X\" type=\"float\"/>"
"              <param name=\"Y\" type=\"float\"/>"
"              <param name=\"Z\" type=\"float\"/>"
"            </accessor>"
"          </technique_common>"
"        </source>"
"        <source id=\"Cube-mesh-normals\">"
"          <float_array id=\"Cube-mesh-normals-array\" count=\"36\">0 0 -1 0 0 1 0.9824244 -4.40568e-7 0.186661 -5.6056e-7 -0.9999946 0.00331521 -0.9824243 2.93712e-7 0.1866608 -0.578491 0.7792385 0.2411128 0 0 -1 0 0 1 0.8888803 -0.407779 0.2088258 0 -0.9999946 0.003315329 -0.9824244 5.6702e-7 0.1866608 2.60346e-7 0.9952206 0.0976516</float_array>"
"          <technique_common>"
"            <accessor source=\"#Cube-mesh-normals-array\" count=\"12\" stride=\"3\">"
"              <param name=\"X\" type=\"float\"/>"
"              <param name=\"Y\" type=\"float\"/>"
"              <param name=\"Z\" type=\"float\"/>"
"            </accessor>"
"          </technique_common>"
"        </source>"
"        <vertices id=\"Cube-mesh-vertices\">"
"          <input semantic=\"POSITION\" source=\"#Cube-mesh-positions\"/>"
"        </vertices>"
"        <polylist material=\"Material-material\" count=\"12\">"
"          <input semantic=\"VERTEX\" source=\"#Cube-mesh-vertices\" offset=\"0\"/>"
"          <input semantic=\"NORMAL\" source=\"#Cube-mesh-normals\" offset=\"1\"/>"
"          <vcount>3 3 3 3 3 3 3 3 3 3 3 3 </vcount>"
"          <p>1 0 2 0 3 0 4 1 7 1 6 1 4 2 5 2 1 2 1 3 5 3 6 3 2 4 6 4 7 4 4 5 0 5 3 5 0 6 1 6 3 6 5 7 4 7 6 7 0 8 4 8 1 8 2 9 1 9 6 9 3 10 2 10 7 10 7 11 4 11 3 11</p>"
"        </polylist>"
"      </mesh>"
"    </geometry>"
"  </library_geometries>"
"  <library_controllers/>"
"  <library_visual_scenes>"
"    <visual_scene id=\"Scene\" name=\"Scene\">"
"      <node id=\"Camera\" name=\"Camera\" type=\"NODE\">"
"        <matrix sid=\"transform\">0.6858805 -0.3173701 0.6548619 7.481132 0.7276338 0.3124686 -0.6106656 -6.50764 -0.01081678 0.8953432 0.4452454 5.343665 0 0 0 1</matrix>"
"        <instance_camera url=\"#Camera-camera\"/>"
"      </node>"
"      <node id=\"Lamp\" name=\"Lamp\" type=\"NODE\">"
"        <matrix sid=\"transform\">-0.2908646 -0.7711008 0.5663932 4.076245 0.9551712 -0.1998834 0.2183912 1.005454 -0.05518906 0.6045247 0.7946723 5.903862 0 0 0 1</matrix>"
"        <instance_light url=\"#Lamp-light\"/>"
"      </node>"
"      <node id=\"Cube\" name=\"Cube\" type=\"NODE\">"
"        <matrix sid=\"transform\">1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1</matrix>"
"        <instance_geometry url=\"#Cube-mesh\">"
"          <bind_material>"
"            <technique_common>"
"              <instance_material symbol=\"Material-material\" target=\"#Material-material\"/>"
"            </technique_common>"
"          </bind_material>"
"        </instance_geometry>"
"      </node>"
"    </visual_scene>"
"  </library_visual_scenes>"
"  <scene>"
"    <instance_visual_scene url=\"#Scene\"/>"
"  </scene>"
"</COLLADA>";

float source1floats[] = 
{ 
  2.607685f, 
  3.291105f, 
  -0.2332705f, 
  1.0f, 
  -0.2133394f, 
  -0.2332705f, 
  -1.0f, 
  -0.2133392f, 
  -0.2332705f, 
  -0.9999997f, 
  0.6128335f, 
  -0.2332705f, 
  0.1993688f, 
  0.1993685f, 
  3.980571f, 
  0.1993686f, 
  -0.1993691f, 
  3.980571f, 
  -0.199369f, 
  -0.1993689f, 
  3.980571f, 
  -0.1993689f, 
  0.1993687f, 
  3.980571f 
};

float source2floats[] = 
{ 
  0.0f, 
  0.0f, 
  -1.0f, 
  0.0f, 
  0.0f, 
  1.0f, 
  0.9824244f, 
  -4.40568e-7f, 
  0.186661f, 
  -5.6056e-7f, 
  -0.9999946f, 
  0.00331521f, 
  -0.9824243f, 
  2.93712e-7f, 
  0.1866608f, 
  -0.578491f, 
  0.7792385f, 
  0.2411128f, 
  0.0f, 
  0.0f, 
  -1.0f, 
  0.0f, 
  0.0f, 
  1.0f, 
  0.8888803f, 
  -0.407779f, 
  0.2088258f, 
  0.0f, 
  -0.9999946f, 
  0.003315329f, 
  -0.9824244f, 
  5.6702e-7f, 
  0.1866608f, 
  2.60346e-7f, 
  0.9952206f, 
  0.0976516f 
};

int pDataIndexes[] = { 1, 0, 2, 0, 3, 0, 4, 1, 7, 1, 6, 1, 4, 2, 5, 2, 1, 2, 1, 3, 5, 3, 6, 3, 2, 4, 6, 4, 7, 4, 4, 5, 0, 5, 3, 5, 0, 6, 1, 6, 3, 6, 5, 7, 4, 7, 6, 7, 0, 8, 4, 8, 1, 8, 2, 9, 1, 9, 6, 9, 3, 10, 2, 10, 7, 10, 7, 11, 4, 11, 3, 11 };

float matrixValues[] = { 1, 0, 0, 0,
                         0, 1, 0, 0,
                         0, 0, 1, 0,
                         0, 0, 0, 1 };

void CheckFloatArray(float * data, float * expected, size_t count)
{
  for (size_t i = 0; i < count; i++)
  {
    CHECK(data[i] == expected[i], );
  }
}

void CheckIntArray(int * data, int * expected, size_t count)
{
  for (size_t i = 0; i < count; i++)
  {
    CHECK(data[i] == expected[i], );
  }
}

void Test_NateMash()
{
  char * newXml1;
  NateMashSource * source;
  NateMashGeometry * geometry;
  NateMashNode * node;
  
  newXml1 = (char*)malloc(strlen(durpMetronome) + 1);
  strcpy_s(newXml1, strlen(durpMetronome) + 1, durpMetronome);

  NateMash * mash = NateMash_Create();
  NateMash_LoadFromColladaData(mash, newXml1, strlen(durpMetronome) + 1, "durpMetronome");

  // verify that 1 geometry was loaded and it has reasonable-looking data
  CHECK(mash->numGeometries == 1, );
  CHECK(mash->geometries != 0, );
  geometry = &mash->geometries[0];
  
  CHECK(geometry != 0, );
  CHECK(strcmp(geometry->id, "Cube-mesh") == 0, );

  // verify that 2 sources were loaded and they have reasonable-looking data
  CHECK(geometry->numSources == 2, );
  CHECK(geometry->sources != 0, );

  source = &geometry->sources[0];
  CHECK(source != 0, );
  CHECK(source->count == 8, );
  CHECK(source->stride == 3, );
  CHECK(source->totalLength == 24, );
  CheckFloatArray(source->data, source1floats, 24);

  source = &geometry->sources[1];
  CHECK(source != 0, );
  CHECK(source->count == 12, );
  CHECK(source->stride == 3, );
  CHECK(source->totalLength == 36, );
  CheckFloatArray(source->data, source2floats, 36);

  CHECK(geometry->polylist.numInputs == 2, );
  CHECK(geometry->polylist.inputs != 0, );
  NateMashPolyListInput * input = &geometry->polylist.inputs[0];
  CHECK(input != 0, );
  CHECK(input->dataType == NateMash_DataType_Vertex, );
  CHECK(input->source == &geometry->sources[0], );
  input = &geometry->polylist.inputs[1];
  CHECK(input != 0, );
  CHECK(input->dataType == NateMash_DataType_Normal, );
  CHECK(input->source == &geometry->sources[1], );

  CHECK(geometry->polylist.numDataIndexes == 12 * 3 * 2, );
  CheckIntArray(geometry->polylist.dataIndexes, pDataIndexes, 12 * 3 * 2);
  CHECK(geometry->polylist.numDataCoordinates == 12, );

  // verify that 1 node was loaded
  CHECK(mash->nodes.nodes != 0, );
  CHECK(mash->nodes.numNodes == 1, );
  node = &mash->nodes.nodes[0];
  CHECK(node != 0, );

  // verify that the node fields look OK
  CHECK(node->geometry != 0, );
  CHECK(node->geometry == geometry, );
  CHECK(strcmp(node->geometryUrl, geometry->id) == 0, );
  CHECK(strcmp(node->id, "Cube") == 0, );
  CHECK(strcmp(node->name, "Cube") == 0, );
  CheckFloatArray(node->transform.elements, matrixValues, 16);
  
  // verify that the node children look OK
  CHECK(node->nodes.nodes == 0, );
  CHECK(node->nodes.numNodes == 0, );
}