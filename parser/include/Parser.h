/* Name: Julian Lau
 * ID: 1054757
 * Email: jlau10@uoguelph.ca
 */
#ifndef PARSER_H
#define PARSER_H

#include <string.h>
#include <strings.h>
#include <math.h>
#include <ctype.h>
#define LIBXML_SCHEMAS_ENABLED
#include <libxml/xmlschemastypes.h>

/* A group of functions which convert an SVG image to a SVGimage struct */
void populateImage(xmlNode * a_node, SVGimage *svgimage); //Links everything and passes it on to creatSVGimage()
void svgParser (xmlNode *cur_node, SVGimage *svgimage); //Parses <svg>
void titleParser (xmlNode *cur_node, SVGimage *svgimage); //Parses <title>
void descParser (xmlNode *cur_node, SVGimage *svgimage); //Parses <desc>
Group *groupParser(xmlNode *cur_node); //Parses <g>
Rectangle *rectParser(xmlNode *cur_node); //Parses <rect>
Circle *circleParser(xmlNode *cur_node); //Parses <circle>
Path *pathParser(xmlNode *cur_node); //Parses <path>

// Helper function for the get"..." functions that traverses the groups recursively to add the specified shape
//0 = rectangles, 1 = circles, 2 = paths, 3 = groups
void traverseGroups(Group *group, List *toAdd, int type);

// Used to free the list allocate by the get"..." functions
void deleteNothing(void *data);

/* A group of functions which converts SVGimage struct to xmlDocPtr */
xmlDocPtr imageToTree(SVGimage *image); //Main function which calls below tree functions and stitches everything together
void svgToTree(xmlNode *node, SVGimage *image); //Helper for xmlDocPtr(), converts <svg>
void rectToTree(xmlNodePtr parent_node, List *rectList); //Helper for xmlDocPtr(), converts <rect>
void circleToTree(xmlNodePtr parent_node, List *circleList); //Helper for xmlDocPtr(), converts <circle>
void pathToTree(xmlNodePtr parent_node, List *pathList); //Helper for xmlDocPtr(), converts <path>
void groupToTree(xmlNodePtr parent_node, List *groupList); //Helper for xmlDocPtr(), converts <g>

/* Validates an SVG image with a schema file by using the tree functions above */
bool validateSVGwithSchema(SVGimage *image, char* schemaFile);

/* Validates an SVG image with the header definitions */
bool validateSVGwithHeader(SVGimage *image);
bool validateRects(List *rectList);
bool validateCircles(List *circleList); 
bool validatePaths(List *pathList);
bool validateGroups(List *groupList);
bool validateAttributes(List *attrList);

/* Helper functions for setAttribute() */
void setSVG(SVGimage* image, elementType elemType, int elemIndex, Attribute* newAttribute);
void setRect(SVGimage* image, elementType elemType, int elemIndex, Attribute* newAttribute);
void setCircle(SVGimage* image, elementType elemType, int elemIndex, Attribute* newAttribute);
void setPath(SVGimage* image, elementType elemType, int elemIndex, Attribute* newAttribute);
void setGroup(SVGimage* image, elementType elemType, int elemIndex, Attribute* newAttribute);

/* Functions wrappers for serverside code */

//Validate File
char* validateSVG(char* fileName);

//Returns a JSON string of an a specified SVG image if its valid.
char* getSVGProperties(char* fileName);
char* getSVGTitleDesc(char* fileName);
char* getSVGRects(char* fileName);
char* getSVGCircs(char* fileName);
char* getSVGPaths(char* fileName);
char* getSVGGroups(char* fileName);

//Changing title/description
char* changeTitle(char* fileName, char* newTitle);
char* changeDescription(char* fileName, char* newDescription);

//Getting attributes
char* getRectAttributes(char* fileName, int num);
char* getCircAttributes(char* fileName, int num);
char* getPathAttributes(char* fileName, int num);
char* getGroupAttributes(char* fileName, int num);

//Editing Attributes
char* editAttributes(char* fileName, int num, char* type, char* attrName, char* attrValue);

//Create SVG
char* createSVG(char* fileName);

//Add shape to SVG
char* addRectangle(char* fileName, char* JSON);
char* addCircle(char* fileName, char* JSON);

//Scale Shape
char* scaleCircles(char* fileName, float scaleFactor);
char* scaleRectangles(char* fileName, float scaleFactor);


#endif
