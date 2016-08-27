/*
This is free and unencumbered software released into the public domain under The Unlicense.
You have complete freedom to do anything you want with the software, for any purpose.
Please refer to <http://unlicense.org/>
*/

#include "TestUtils.h"
#include "NateXml.h"
#include "SDL.h"

#pragma warning(disable : 4127) // conditional expression is constant
#pragma warning(disable : 4100) // unreferenced formal parameter

char * xml1 = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n"
  "<Setup Id=\"VisualWebDeveloper\" Type=\"Product\" ProductCode=\"{90120000-0021-0000-0000-0000000FF1CE}\">\r"
  "  <PIDTemplate Value=\"825&#34;&u1234;03&amp;&lt;````=````=````=````=`````&amp;&gt;@@@@@\"/>\n"
	"  <Option Id=\"AlwaysInstalled\" DefaultState=\"Local\" DisallowAbsent=\"yes\" DisallowAdvertise=\"yes\" Hidden=\"yes\"/>\n"
	"\t<PackageRef Path=\"VisualWebDeveloperWW.xml\">\r"
  "\t  <SkuType Value=\"standalone\">Steve</SkuType>\n"
  "    <DPC>Charlie</DPC>\r\n"
  "    <DPC><!--huh-->Cha<!--ok-->rl<!--why?-->ie<!--fine--></DPC>\r\n"
  "    <Huh/>\n"
  "  </PackageRef>\n"
  "</Setup>";

void Xml1_Callback1(char * elementName, char * attributes, int attributeCount, char * elementText, int depth, void * userData)
{
  // test value: "<Setup Id=\"VisualWebDeveloper\" Type=\"Product\" ProductCode=\"{90120000-0021-0000-0000-0000000FF1CE}\">\r"
  char * names[3];
  char * values[3];

  CHECK(strcmp(elementName, "Setup") == 0, );
  CHECK(attributeCount == 3, );

  NateXml_GetAttributes(attributes, 3, names, values);
  CHECK(strcmp(names[0], "Id") == 0, );
  CHECK(strcmp(values[0], "VisualWebDeveloper") == 0, );
  CHECK(strcmp(names[1], "Type") == 0, );
  CHECK(strcmp(values[1], "Product") == 0, );
  CHECK(strcmp(names[2], "ProductCode") == 0, );
  CHECK(strcmp(values[2], "{90120000-0021-0000-0000-0000000FF1CE}") == 0, );

  CHECK(strcmp(elementText, "\r  ") == 0, );
  CHECK(depth == 1, );
  CHECK((int)userData == 3, );
}

void Xml1_Callback2(char * elementName, char * attributes, int attributeCount, char * elementText, int depth, void * userData)
{
  // test value: "  <PIDTemplate Value=\"825&#34;&u1234;03&amp;&lt;````=````=````=````=`````&amp;&gt;@@@@@\"/>\n"
  char * names[1];
  char * values[1];

  CHECK(strcmp(elementName, "PIDTemplate") == 0, );
  CHECK(attributeCount == 1, );

  NateXml_GetAttributes(attributes, 1, names, values);
  CHECK(strcmp(names[0], "Value") == 0, );
  CHECK(strcmp(values[0], "825\"?03&<````=````=````=````=`````&>@@@@@") == 0, );

  CHECK(strcmp(elementText, "") == 0, );
  CHECK(depth == 2, );
  CHECK((int)userData == 3, );
}

void Xml1_Callback3(char * elementName, char * attributes, int attributeCount, char * elementText, int depth, void * userData)
{
	// test value: "  <Option Id=\"AlwaysInstalled\" DefaultState=\"Local\" DisallowAbsent=\"yes\" DisallowAdvertise=\"yes\" Hidden=\"yes\"/>\n"
  char * names[5];
  char * values[5];

  CHECK(strcmp(elementName, "Option") == 0, );
  CHECK(attributeCount == 5, );

  NateXml_GetAttributes(attributes, 5, names, values);
  CHECK(strcmp(names[0], "Id") == 0, );
  CHECK(strcmp(values[0], "AlwaysInstalled") == 0, );
  CHECK(strcmp(names[1], "DefaultState") == 0, );
  CHECK(strcmp(values[1], "Local") == 0, );
  CHECK(strcmp(names[2], "DisallowAbsent") == 0, );
  CHECK(strcmp(values[2], "yes") == 0, );
  CHECK(strcmp(names[3], "DisallowAdvertise") == 0, );
  CHECK(strcmp(values[3], "yes") == 0, );
  CHECK(strcmp(names[4], "Hidden") == 0, );
  CHECK(strcmp(values[4], "yes") == 0, );

  CHECK(strcmp(elementText, "") == 0, );
  CHECK(depth == 2, );
  CHECK((int)userData == 3, );
}

void Xml1_Callback4(char * elementName, char * attributes, int attributeCount, char * elementText, int depth, void * userData)
{
	// test value: "\t<PackageRef Path=\"VisualWebDeveloperWW.xml\">\r"
  char * names[1];
  char * values[1];

  CHECK(strcmp(elementName, "PackageRef") == 0, );
  CHECK(attributeCount == 1, );

  NateXml_GetAttributes(attributes, 1, names, values);
  CHECK(strcmp(names[0], "Path") == 0, );
  CHECK(strcmp(values[0], "VisualWebDeveloperWW.xml") == 0, );

  CHECK(strcmp(elementText, "\r\t  ") == 0, );
  CHECK(depth == 2, );
  CHECK((int)userData == 3, );
}

void Xml1_Callback5(char * elementName, char * attributes, int attributeCount, char * elementText, int depth, void * userData)
{
  // test value: "\t  <SkuType Value=\"standalone\">Steve</SkuType>\n"
  char * names[1];
  char * values[1];

  CHECK(strcmp(elementName, "SkuType") == 0, );
  CHECK(attributeCount == 1, );

  NateXml_GetAttributes(attributes, 1, names, values);
  CHECK(strcmp(names[0], "Value") == 0, );
  CHECK(strcmp(values[0], "standalone") == 0, );

  CHECK(strcmp(elementText, "Steve") == 0, );
  CHECK(depth == 3, );
  CHECK((int)userData == 3, );
}

void Xml1_Callback6(char * elementName, char * attributes, int attributeCount, char * elementText, int depth, void * userData)
{
  // test value: "    <DPC>Charlie</DPC>\r\n"
  CHECK(strcmp(elementName, "DPC") == 0, );
  CHECK(attributeCount == 0, );
  CHECK(attributes == 0, );
  CHECK(strcmp(elementText, "Charlie") == 0, );
  CHECK(depth == 3, );
  CHECK((int)userData == 3, );
}

void Xml1_Callback7(char * elementName, char * attributes, int attributeCount, char * elementText, int depth, void * userData)
{
  // test value: "    <DPC><!--huh-->Cha<!--ok-->rl<!--why?-->ie<!--fine--></DPC>\r\n"
  CHECK(strcmp(elementName, "DPC") == 0, );
  CHECK(attributeCount == 0, );
  CHECK(attributes == 0, );
  CHECK(strcmp(elementText, "Charlie") == 0, );
  CHECK(depth == 3, );
  CHECK((int)userData == 3, );
}

void Xml1_Callback8(char * elementName, char * attributes, int attributeCount, char * elementText, int depth, void * userData)
{
  // test value: "    <Huh/>\n"
  CHECK(strcmp(elementName, "Huh") == 0, );
  CHECK(attributeCount == 0, );
  CHECK(attributes == 0, );
  CHECK(strcmp(elementText, "") == 0, );
  CHECK(depth == 3, );
  CHECK((int)userData == 3, );
}

int callbackNumber = 0;

void Xml1_MainCallback(char * elementName, char * attributes, size_t attributeCount, char * elementText, size_t depth, void * userData)
{
  callbackNumber++;
  switch (callbackNumber)
  {
    case 1:
      Xml1_Callback1(elementName, attributes, attributeCount, elementText, depth, userData);
      break;

    case 2:
      Xml1_Callback2(elementName, attributes, attributeCount, elementText, depth, userData);
      break;

    case 3:
      Xml1_Callback3(elementName, attributes, attributeCount, elementText, depth, userData);
      break;

    case 4:
      Xml1_Callback4(elementName, attributes, attributeCount, elementText, depth, userData);
      break;

    case 5:
      Xml1_Callback5(elementName, attributes, attributeCount, elementText, depth, userData);
      break;

    case 6:
      Xml1_Callback6(elementName, attributes, attributeCount, elementText, depth, userData);
      break;

    case 7:
      Xml1_Callback7(elementName, attributes, attributeCount, elementText, depth, userData);
      break;

    case 8:
      Xml1_Callback8(elementName, attributes, attributeCount, elementText, depth, userData);
      break;

    default:
      CHECK(0, "Too many callbacks for Xml1");
  }
}

void Test_NateXml_Streaming()
{
  int result;
  char * newXml1;
  char errorBuffer[1024];
  
  callbackNumber = 0;
  newXml1 = malloc(strlen(xml1) + 1);
  strcpy(newXml1, xml1);
  result = NateXml_ParseStreaming(newXml1, strlen(xml1), errorBuffer, sizeof(errorBuffer), (void*)3, Xml1_MainCallback);
  free(newXml1);
  
  CHECK(result == 1, "Xml1 failed");
}

void XmlDomCallback(NateXmlNode * fakeRootNode, void * userData)
{
  char * names[10];
  char * values[10];
  NateXmlNode * root;
  NateXmlNode * node;
  NateXmlNode * child;

  /*
char * xml1 = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n"
  "<Setup Id=\"VisualWebDeveloper\" Type=\"Product\" ProductCode=\"{90120000-0021-0000-0000-0000000FF1CE}\">\r"
  "  <PIDTemplate Value=\"825&#34;&u1234;03&amp;&lt;````=````=````=````=`````&amp;&gt;@@@@@\"/>\n"
	"  <Option Id=\"AlwaysInstalled\" DefaultState=\"Local\" DisallowAbsent=\"yes\" DisallowAdvertise=\"yes\" Hidden=\"yes\"/>\n"
	"\t<PackageRef Path=\"VisualWebDeveloperWW.xml\">\r"
  "\t  <SkuType Value=\"standalone\">Steve</SkuType>\n"
  "    <DPC>Charlie</DPC>\r\n"
  "    <DPC><!--huh-->Cha<!--ok-->rl<!--why?-->ie<!--fine--></DPC>\r\n"
  "    <Huh/>\n"
  "  </PackageRef>\n"
  "</Setup>";
  */
  CHECK(NateXmlNode_GetCount(fakeRootNode) == 1, );
  root = NateXmlNode_GetChild(fakeRootNode, 0);
  CHECK(root != 0, );
  CHECK(strcmp(root->ElementName, "Setup") == 0, );
  CHECK(root->AttributeCount == 3, );
  NateXml_GetAttributes(root->Attributes, 3, names, values);
  CHECK(strcmp(names[0], "Id") == 0, );
  CHECK(strcmp(values[0], "VisualWebDeveloper") == 0, );
  CHECK(strcmp(names[1], "Type") == 0, );
  CHECK(strcmp(values[1], "Product") == 0, );
  CHECK(strcmp(names[2], "ProductCode") == 0, );
  CHECK(strcmp(values[2], "{90120000-0021-0000-0000-0000000FF1CE}") == 0, );
  CHECK(strcmp(root->ElementText, "\r  ") == 0, );
  CHECK(root->Depth == 1, );
  CHECK(root->UserData == 0, );
  CHECK(NateXmlNode_GetCount(root) == 3, );
  CHECK(root->Parent == fakeRootNode, );

  node = NateXmlNode_GetChild(root, 0);
  CHECK(node != 0, );
  CHECK(strcmp(node->ElementName, "PIDTemplate") == 0, );
  CHECK(node->AttributeCount == 1, );
  NateXml_GetAttributes(node->Attributes, 1, names, values);
  CHECK(strcmp(names[0], "Value") == 0, );
  CHECK(strcmp(values[0], "825\"?03&<````=````=````=````=`````&>@@@@@") == 0, );
  CHECK(strcmp(node->ElementText, "") == 0, );
  CHECK(node->Depth == 2, );
  CHECK(node->UserData == 0, );
  CHECK(NateXmlNode_GetCount(node) == 0, );
  CHECK(node->Parent == root, );

  node = NateXmlNode_GetChild(root, 1);
  CHECK(node != 0, );
  CHECK(strcmp(node->ElementName, "Option") == 0, );
  CHECK(node->AttributeCount == 5, );
  NateXml_GetAttributes(node->Attributes, 5, names, values);
  CHECK(strcmp(names[0], "Id") == 0, );
  CHECK(strcmp(values[0], "AlwaysInstalled") == 0, );
  CHECK(strcmp(names[1], "DefaultState") == 0, );
  CHECK(strcmp(values[1], "Local") == 0, );
  CHECK(strcmp(names[2], "DisallowAbsent") == 0, );
  CHECK(strcmp(values[2], "yes") == 0, );
  CHECK(strcmp(names[3], "DisallowAdvertise") == 0, );
  CHECK(strcmp(values[3], "yes") == 0, );
  CHECK(strcmp(names[4], "Hidden") == 0, );
  CHECK(strcmp(values[4], "yes") == 0, );
  CHECK(strcmp(node->ElementText, "") == 0, );
  CHECK(node->Depth == 2, );
  CHECK(node->UserData == 0, );
  CHECK(NateXmlNode_GetCount(node) == 0, );
  CHECK(node->Parent == root, );

  node = NateXmlNode_GetChild(root, 2);
  CHECK(node != 0, );
  CHECK(strcmp(node->ElementName, "PackageRef") == 0, );
  CHECK(node->AttributeCount == 1, );
  NateXml_GetAttributes(node->Attributes, 1, names, values);
  CHECK(strcmp(names[0], "Path") == 0, );
  CHECK(strcmp(values[0], "VisualWebDeveloperWW.xml") == 0, );
  CHECK(strcmp(node->ElementText, "\r\t  ") == 0, );
  CHECK(node->Depth == 2, );
  CHECK(node->UserData == 0, );
  CHECK(NateXmlNode_GetCount(node) == 4, );
  CHECK(node->Parent == root, );

  /*
    "\t  <SkuType Value=\"standalone\">Steve</SkuType>\n"
  "    <DPC>Charlie</DPC>\r\n"
  "    <DPC><!--huh-->Cha<!--ok-->rl<!--why?-->ie<!--fine--></DPC>\r\n"
  "    <Huh/>\n"
  */
  child = NateXmlNode_GetChild(node, 0);
  CHECK(child != 0, );
  CHECK(strcmp(child->ElementName, "SkuType") == 0, );
  CHECK(child->AttributeCount == 1, );
  NateXml_GetAttributes(child->Attributes, 1, names, values);
  CHECK(strcmp(names[0], "Value") == 0, );
  CHECK(strcmp(values[0], "standalone") == 0, );
  CHECK(strcmp(child->ElementText, "Steve") == 0, );
  CHECK(child->Depth == 3, );
  CHECK(child->UserData == 0, );
  CHECK(NateXmlNode_GetCount(child) == 0, );
  CHECK(child->Parent == node, );

  child = NateXmlNode_GetChild(node, 1);
  CHECK(child != 0, );
  CHECK(strcmp(child->ElementName, "DPC") == 0, );
  CHECK(child->AttributeCount == 0, );
  CHECK(strcmp(child->ElementText, "Charlie") == 0, );
  CHECK(child->Depth == 3, );
  CHECK(child->UserData == 0, );
  CHECK(NateXmlNode_GetCount(child) == 0, );
  CHECK(child->Parent == node, );

  child = NateXmlNode_GetChild(node, 2);
  CHECK(child != 0, );
  CHECK(strcmp(child->ElementName, "DPC") == 0, );
  CHECK(child->AttributeCount == 0, );
  CHECK(strcmp(child->ElementText, "Charlie") == 0, );
  CHECK(child->Depth == 3, );
  CHECK(child->UserData == 0, );
  CHECK(NateXmlNode_GetCount(child) == 0, );
  CHECK(child->Parent == node, );

  child = NateXmlNode_GetChild(node, 3);
  CHECK(child != 0, );
  CHECK(strcmp(child->ElementName, "Huh") == 0, );
  CHECK(child->AttributeCount == 0, );
  CHECK(strcmp(child->ElementText, "") == 0, );
  CHECK(child->Depth == 3, );
  CHECK(child->UserData == 0, );
  CHECK(NateXmlNode_GetCount(child) == 0, );
  CHECK(child->Parent == node, );
}

void Test_NateXml_Dom()
{
  int result;
  char * newXml1;
  char errorBuffer[1024];
  
  newXml1 = malloc(strlen(xml1) + 1);
  strcpy(newXml1, xml1);
  result = NateXml_ParseDom(newXml1, strlen(xml1), errorBuffer, sizeof(errorBuffer), (void*)3, XmlDomCallback);
  free(newXml1);
  
  CHECK(result == 1, "Xml1 failed");
}

void Test_NateXml()
{
  Test_NateXml_Streaming();
  Test_NateXml_Dom();
}