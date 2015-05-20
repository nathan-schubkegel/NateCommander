#include "TestUtils.h"
#include "ccan/NateXml/NateXml.h"
#include "SDL.h"

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

void Xml1_MainCallback(char * elementName, char * attributes, int attributeCount, char * elementText, int depth, void * userData)
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

void Test_NateXml()
{
  int result;
  char * newXml1;
  char errorBuffer[1024];
  
  callbackNumber = 0;
  newXml1 = malloc(strlen(xml1) + 1);
  strcpy(newXml1, xml1);
  result = NateXml_ParseFile(newXml1, strlen(xml1), errorBuffer, sizeof(errorBuffer), (void*)3, Xml1_MainCallback);
  
  CHECK(result == 1, "Xml1 failed");
}