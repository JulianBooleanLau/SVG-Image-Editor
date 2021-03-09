/* Name: Julian Lau
 * ID: 1054757
 * Email: jlau10@uoguelph.ca
 */
#include "SVGParser.h"
#include "Parser.h"

/* Public API - main */

/* A group of functions which convert an SVG image to a SVGimage struct */
SVGimage* createSVGimage(char* fileName){
	/*Variables*/
	SVGimage *svgimage = malloc(sizeof(SVGimage));
	xmlDoc *doc = NULL;
    xmlNode *root_element = NULL;
	
	/*Setting up the XML parser*/
    LIBXML_TEST_VERSION
    doc = xmlReadFile(fileName, NULL, 0);
    if (doc == NULL) {
		free(svgimage);
		xmlFreeDoc(doc);
		xmlCleanupParser();
		return NULL;
    } else {
		root_element = xmlDocGetRootElement(doc);

		/*Setting up default values and parser*/
		strcpy(svgimage->namespace, "");
		strcpy(svgimage->title, "");
		strcpy(svgimage->description, "");
		svgimage->rectangles = initializeList(&rectangleToString, &deleteRectangle, &compareRectangles);
		svgimage->circles = initializeList(&circleToString, &deleteCircle, &compareCircles);
		svgimage->paths = initializeList(&pathToString, &deletePath, &comparePaths);
		svgimage->groups = initializeList(&groupToString, &deleteGroup, &compareGroups);
		svgimage->otherAttributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);
		populateImage(root_element, svgimage); //Helper function to populate new svgimage
		
		//Cleaning up
		xmlFreeDoc(doc);
		xmlCleanupParser();
	}
  
    return svgimage;
}
void populateImage(xmlNode * a_node, SVGimage *svgimage) {
	if (svgimage == NULL || a_node == NULL) {
		return;
	}
	
    xmlNode *cur_node = NULL;
    for (cur_node = a_node; cur_node != NULL; cur_node = cur_node->next) {
		//printImage(cur_node);
		
		//Checks the currents nodes name and adds it to the image
		if (strcasecmp((char *)cur_node->name, "svg") == 0) { //Nodes is svg
			svgParser(cur_node, svgimage);
			populateImage(cur_node->children, svgimage);
		} else if (strcasecmp((char *)cur_node->name, "title") == 0) { //Node is title
			titleParser(cur_node, svgimage);
			populateImage(cur_node->children, svgimage);
		} else if (strcasecmp((char *)cur_node->name, "desc") == 0) { //Node is decription
			descParser(cur_node, svgimage);
			populateImage(cur_node->children, svgimage);
		} else if (strcasecmp((char *)cur_node->name, "rect") == 0) { //Node is rectangle
			insertBack(svgimage->rectangles, rectParser(cur_node));
			populateImage(cur_node->children, svgimage);
		} else if (strcasecmp((char *)cur_node->name, "circle") == 0) { //Node is circle
			insertBack(svgimage->circles, circleParser(cur_node));
			populateImage(cur_node->children, svgimage);
		} else if (strcasecmp((char *)cur_node->name, "path") == 0) { //Node is a path
			insertBack(svgimage->paths, pathParser(cur_node));
			populateImage(cur_node->children, svgimage);
		} else if (strcasecmp((char *)cur_node->name, "g") == 0) { //Node is group. This is special as groups can be in groups
			insertBack(svgimage->groups, groupParser(cur_node));
			//It recursively calls itself in the function
		} 
    }
}
void svgParser(xmlNode *cur_node, SVGimage *svgimage) {
	if (cur_node->ns != NULL && svgimage != NULL && cur_node != NULL) {
		snprintf(svgimage->namespace, 256, "%s", (char *)cur_node->ns->href);
		//Other attributes
		xmlAttr *attr;
		for (attr = cur_node->properties; attr != NULL; attr = attr->next) {
			xmlNode *value = attr->children;
			char *attrName = (char *)(attr->name);
			char *attrStr = (char *)(value->content);
			Attribute *tempAttr = malloc(sizeof(Attribute));
			tempAttr->name = malloc(strlen(attrName) + 4);
			tempAttr->value = malloc(strlen(attrStr) + 4);
			strcpy(tempAttr->name, attrName);
			strcpy(tempAttr->value, attrStr);
			insertBack(svgimage->otherAttributes, tempAttr);
		}
	}
}
void titleParser(xmlNode *cur_node, SVGimage *svgimage) {
	if (cur_node != NULL && cur_node->children != NULL && cur_node->children->content != NULL  && svgimage != NULL) {
		snprintf(svgimage->title, 256, "%s", (char *)cur_node->children->content);
		//printf("	title: %s\n", svgimage->title);
	}
}
void descParser(xmlNode *cur_node, SVGimage *svgimage) {
	if (cur_node != NULL && cur_node->children != NULL && cur_node->children->content != NULL && svgimage != NULL) {
		snprintf(svgimage->description, 256, "%s", (char *)cur_node->children->content);
		//printf("	desc: %s\n", svgimage->description);
	}
}
Group *groupParser(xmlNode *cur_node) {
	//Initalizing the group being added
	Group *tempGroup = malloc(sizeof(Group));
	tempGroup->rectangles = initializeList(&rectangleToString, &deleteRectangle, &compareRectangles);
	tempGroup->circles = initializeList(&circleToString, &deleteCircle, &compareCircles);
	tempGroup->paths = initializeList(&pathToString, &deletePath, &comparePaths);
	tempGroup->groups = initializeList(&groupToString, &deleteGroup, &compareGroups);
	tempGroup->otherAttributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);

	//Looping through Group's Children
	xmlNode *children = NULL;
	for (children = cur_node->children; children != NULL; children = children->next) {
		if (strcasecmp((char *)children->name, "rect") == 0) { //Node is rectangle
			insertBack(tempGroup->rectangles, rectParser(children));
		} else if (strcasecmp((char *)children->name, "circle") == 0) { //Node is circle
			insertBack(tempGroup->circles, circleParser(children));
		} else if (strcasecmp((char *)children->name, "path") == 0) { //Node is a path
			insertBack(tempGroup->paths, pathParser(children));
		} else if (strcasecmp((char *)children->name, "g") == 0) { //Node is group
			insertBack(tempGroup->groups, groupParser(children)); //Which then recursively calls itself
		}
	}
	
	//Adding Other Attributes
	xmlAttr *attr;
    for (attr = cur_node->properties; attr != NULL; attr = attr->next) {
        xmlNode *value = attr->children;
		char *attrName = (char *)(attr->name);
        char *attrStr = (char *)(value->content);
        
        Attribute *tempAttr = malloc(sizeof(Attribute));
		tempAttr->name = malloc(strlen(attrName) + 1);
		tempAttr->value = malloc(strlen(attrStr) + 1);
		strcpy(tempAttr->name, attrName);
		strcpy(tempAttr->value, attrStr);
		insertBack(tempGroup->otherAttributes, tempAttr);
	}
	
	return tempGroup;
}
Rectangle *rectParser(xmlNode *cur_node) {		
	Rectangle *tempRect = malloc(sizeof(Rectangle));
	strcpy(tempRect->units, "");
	tempRect->otherAttributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);
	
	xmlAttr *attr;
    for (attr = cur_node->properties; attr != NULL; attr = attr->next) {
        xmlNode *value = attr->children;
		char *attrName = (char *)(attr->name);
        char *attrStr = (char *)(value->content);
        
        //Parses the attribute value (float) and its units (char *)
        char nums[100];
        char units[50];
        int i, numsI, unitsI;
		strcpy(nums, "");
		strcpy(units, "");
        i = numsI = unitsI = 0;
        for (i = 0; i < strlen(attrStr); ++i) {
			if ((attrStr[i] >= '0' && attrStr[i] <= '9') || attrStr[i] == '.' || attrStr[i] == '-') {
				nums[numsI] = attrStr[i];
				++numsI;
			} else if (attrStr[i] >= 'a' && attrStr[i] <= 'z') {
				units[unitsI] = attrStr[i];
				++unitsI;
			}
		}
		nums[numsI] = '\0';
		units[unitsI] = '\0';
		
		//Assigning values to the rect
        if (strcasecmp(attrName, "x") == 0) {
			tempRect->x = (float)atof(nums);
			if (strcasecmp(units, "") != 0) { //Units
				strcpy(tempRect->units, units);
			}
		} else if (strcasecmp(attrName, "y") == 0) {
			tempRect->y = (float)atof(nums);
		} else if (strcasecmp(attrName, "width") == 0) {
			tempRect->width = (float)atof(nums);
		} else if (strcasecmp(attrName, "height") == 0) {
			tempRect->height = (float)atof(nums);	
		} else { //Other attributes
			Attribute *tempAttr = malloc(sizeof(Attribute));
			tempAttr->name = malloc(strlen(attrName) + 1);
			tempAttr->value = malloc(strlen(attrStr) + 1);
			strcpy(tempAttr->name, attrName);
			strcpy(tempAttr->value, attrStr);
			insertBack(tempRect->otherAttributes, tempAttr);
		}
		
	}
	return tempRect;
	//insertBack(svgimage->rectangles, (void*)tempRect);
}
Circle *circleParser(xmlNode *cur_node) {
	Circle *tempCircle = malloc(sizeof(Circle));
	strcpy(tempCircle->units, "");
	tempCircle->otherAttributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);

	xmlAttr *attr;
    for (attr = cur_node->properties; attr != NULL; attr = attr->next) {
        xmlNode *value = attr->children;
		char *attrName = (char *)(attr->name);
        char *attrStr = (char *)(value->content);
        
        //Parses the attribute value (float) and its units (char *)
        char nums[100];
        char units[50];
        int i, numsI, unitsI;
		strcpy(nums, "");
		strcpy(units, "");
        i = numsI = unitsI = 0;
        for (i = 0; i < strlen(attrStr); ++i) {
			if ((attrStr[i] >= '0' && attrStr[i] <= '9') || attrStr[i] == '.' || attrStr[i] == '-') {
				nums[numsI] = attrStr[i];
				++numsI;
			} else if (attrStr[i] >= 'a' && attrStr[i] <= 'z') {
				units[unitsI] = attrStr[i];
				++unitsI;
			}
		}
		nums[numsI] = '\0';
		units[unitsI] = '\0';
		
		//Assigning values to the rect
        if (strcasecmp(attrName, "cx") == 0) { //cx
			tempCircle->cx = (float)atof(nums);
			strcpy(tempCircle->units, units); //Units
		} else if (strcasecmp(attrName, "cy") == 0) { //cy
			tempCircle->cy = (float)atof(nums);
		} else if (strcasecmp(attrName, "r") == 0) { //r
			tempCircle->r = (float)atof(nums);
		} else { //Other attributes
			Attribute *tempAttr = malloc(sizeof(Attribute));
			tempAttr->name = malloc(strlen(attrName) + 1);
			tempAttr->value = malloc(strlen(attrStr) + 1);
			strcpy(tempAttr->name, attrName);
			strcpy(tempAttr->value, attrStr);
			insertBack(tempCircle->otherAttributes, tempAttr);
		}
	}
	return tempCircle;
}
Path *pathParser(xmlNode *cur_node) {
	Path *tempPath = malloc(sizeof(Path));
	tempPath->data = malloc(1);
	strcpy(tempPath->data, "");
	tempPath->otherAttributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);

	xmlAttr *attr;
    for (attr = cur_node->properties; attr != NULL; attr = attr->next) {
        xmlNode *value = attr->children;
		char *attrName = (char *)(attr->name);
        char *attrStr = (char *)(value->content);
		
		//Assigning values to the path
        if (strcasecmp(attrName, "d") == 0) { //d
			tempPath->data = realloc(tempPath->data, strlen(attrStr) + 1);
			strcpy(tempPath->data, attrStr);
		} else { //Other attributes
			Attribute *tempAttr = malloc(sizeof(Attribute));
			tempAttr->name = malloc(strlen(attrName) + 1);
			tempAttr->value = malloc(strlen(attrStr) + 1);
			strcpy(tempAttr->name, attrName);
			strcpy(tempAttr->value, attrStr);
			insertBack(tempPath->otherAttributes, tempAttr);
		}
	}
	return tempPath;
}

char* SVGimageToString(SVGimage* img) {
	if (img == NULL) {
		return NULL;
	}
	
	ListIterator iter;
	void *elem;
	char *str;
	
	char *string = malloc(strlen(img->namespace) + strlen(img->title) + strlen(img->description) + 100);
	sprintf(string, "Namespace: %s \nTitle: %s \nDescription: %s \n", img->namespace, img->title, img->description);

	//Adding SVG to string
	strcat(string, "SVG:\n\t");
	iter = createIterator(img->otherAttributes);
	while ((elem = nextElement(&iter)) != NULL) {
		Attribute *tempAttr = (Attribute *)elem;
		str = img->otherAttributes->printData(tempAttr);
		string = realloc(string, strlen(string) + strlen(str) + 16);
		strcat(string, str);
		strcat(string, " ");
		free(str);
	}
	strcat(string, "\n");
	
	//Adding Rectangles to string
	iter = createIterator(img->rectangles);
	while ((elem = nextElement(&iter)) != NULL) {
		Rectangle *tempRect = (Rectangle *)elem;
		str = img->rectangles->printData(tempRect);
		string = realloc(string, strlen(string) + strlen(str) + 16);
		strcat(string, str);
		free(str);
	}
	
	//Adding Circles to string
	iter = createIterator(img->circles);
	while ((elem = nextElement(&iter)) != NULL) {
		Circle *tempCircle = (Circle *)elem;
		str = img->circles->printData(tempCircle);
		string = realloc(string, strlen(string) + strlen(str) + 16);
		strcat(string, str);
		free(str);
	}
	
	//Adding Paths to string
	iter = createIterator(img->paths);
	while ((elem = nextElement(&iter)) != NULL) {
		Path *tempPath = (Path *)elem;
		str = img->paths->printData(tempPath);
		string = realloc(string, strlen(string) + strlen(str) + 16);
		strcat(string, str);
		free(str);
	}
	
	//Adding Groups to string
	iter = createIterator(img->groups);
	while ((elem = nextElement(&iter)) != NULL) {
		Group *tempGroup = (Group *)elem;
		str = img->groups->printData(tempGroup);
		string = realloc(string, strlen(string) + strlen(str) + 16);
		strcat(string, str);
		free(str);
	}
	
	return string;
}

void deleteSVGimage(SVGimage* img) {
	if (img != NULL) {
		freeList(img->rectangles);
		freeList(img->circles);
		freeList(img->paths);
		freeList(img->groups);
		freeList(img->otherAttributes);
		free(img);
	}
}

List* getRects(SVGimage* img) {
	List *rectList = initializeList(&rectangleToString, &deleteNothing, &compareRectangles);
	if (img == NULL) {
		return rectList;
	}
	void *elem;
	ListIterator iter;
	
	//Checking nomral rectangles
	iter = createIterator(img->rectangles);
	while ((elem = nextElement(&iter)) != NULL) {
		insertBack(rectList, (Rectangle *)elem);
	}
	
	//Checking rectangles in groups
	iter = createIterator(img->groups);
	while ((elem = nextElement(&iter)) != NULL) {
		traverseGroups((Group *)elem, rectList, 0);
	}

	return rectList;
}
List* getCircles(SVGimage* img) {
	List *circleList = initializeList(&circleToString, &deleteNothing, &compareCircles);
	if (img == NULL) {
		return circleList;
	}
	void *elem;
	ListIterator iter;
	
	//Checking nomral circles
	iter = createIterator(img->circles);
	while ((elem = nextElement(&iter)) != NULL) {
		insertBack(circleList, (Circle *)elem);
	}
	
	//Checking rectangles in groups
	iter = createIterator(img->groups);
	while ((elem = nextElement(&iter)) != NULL) {
		traverseGroups((Group *)elem, circleList, 1);
	}

	return circleList;
}
List* getPaths(SVGimage* img) {
	List *pathList = initializeList(&pathToString, &deleteNothing, &comparePaths);
	if (img == NULL) {
		return pathList;
	}
	void *elem;
	ListIterator iter;
	
	//Checking nomral paths
	iter = createIterator(img->paths);
	while ((elem = nextElement(&iter)) != NULL) {
		insertBack(pathList, (Path *)elem);
	}
	
	//Checking paths in groups
	iter = createIterator(img->groups);
	while ((elem = nextElement(&iter)) != NULL) {
		traverseGroups((Group *)elem, pathList, 2);
	}

	return pathList;
}
List* getGroups(SVGimage* img) {
	List *groupList = initializeList(&groupToString, &deleteNothing, &compareGroups);
	if (img == NULL) {
		return groupList;
	}
	void *elem;
	ListIterator iter;

	//Checking groups + group in groups
	iter = createIterator(img->groups);
	while ((elem = nextElement(&iter)) != NULL) {
		insertBack(groupList, (Group *)elem);
		traverseGroups((Group *)elem, groupList, 3);
	}

	return groupList;
}

//Helper function that deletes nothing and is used to free the list created by the getter functions.
void deleteNothing(void *data) {
	return;
}


// Helper function for the get"..." functions that traverses the groups recursively to add the specified shape
//0 = rectangles, 1 = circles, 2 = paths, 3 = groups
void traverseGroups(Group *group, List *toAdd, int type) {
	if (group == NULL || toAdd == NULL) {
		return;
	}
	void *elem;
	ListIterator iter;
	
	if (type == 0) { //Rectangle
		//Rectangles in the group
		iter = createIterator(group->rectangles);
		while ((elem = nextElement(&iter)) != NULL) {
			insertBack(toAdd, (Rectangle *)elem);
		}
		//Groups within groups + recursion
		iter = createIterator(group->groups);
		while ((elem = nextElement(&iter)) != NULL) {
			traverseGroups((Group *)elem, toAdd, 0);
		}
	} else if (type == 1) {
		//Circles in the group
		iter = createIterator(group->circles);
		while ((elem = nextElement(&iter)) != NULL) {
			insertBack(toAdd, (Circle *)elem);
		}
		//Groups within groups + recursion
		iter = createIterator(group->groups);
		while ((elem = nextElement(&iter)) != NULL) {
			traverseGroups((Group *)elem, toAdd, 1);
		}
	} else if (type == 2) {
		//Paths in the group
		iter = createIterator(group->paths);
		while ((elem = nextElement(&iter)) != NULL) {
			insertBack(toAdd, (Path *)elem);
		}
		//Groups within groups + recursion
		iter = createIterator(group->groups);
		while ((elem = nextElement(&iter)) != NULL) {
			traverseGroups((Group *)elem, toAdd, 2);
		}
	} else if (type == 3) {
		//Groups within groups + recursion
		iter = createIterator(group->groups);
		while ((elem = nextElement(&iter)) != NULL) {
			insertBack(toAdd, (Group *)elem);
			traverseGroups((Group *)elem, toAdd, 3);
		}
	}
}

int numRectsWithArea(SVGimage* img, float area) {
	if (img == NULL || area < 0) {
		return 0;
	}
	
	int sum = 0;
	void *elem;
	List *rectList = getRects(img);
	ListIterator iter = createIterator(rectList);
	while ((elem = nextElement(&iter)) != NULL) {
		Rectangle *tempRect = (Rectangle *)elem;
		float rectArea = tempRect->width * tempRect->height;
		if (ceilf(area) == ceilf(rectArea)) {
			sum++;
		}
	}
	freeList(rectList);
	return sum;
}
int numCirclesWithArea(SVGimage* img, float area) {
	if (img == NULL || area < 0) {
		return 0;
	}
	
	int sum = 0;
	void *elem;
	List *circleList = getCircles(img);
	ListIterator iter = createIterator(circleList);
	while ((elem = nextElement(&iter)) != NULL) {
		Circle *tempCircle = (Circle *)elem;
		float circleArea = powf(tempCircle->r, 2) * 3.14159;
		if (ceilf(area) == ceilf(circleArea)) {
			sum++;
		}
	}
	freeList(circleList);
	return sum;
}
int numPathsWithdata(SVGimage* img, char* data) {
	if (img == NULL || data == NULL) {
		return 0;
	}
	
	int sum = 0;
	void *elem;
	List *pathList = getPaths(img);
	ListIterator iter = createIterator(pathList);
	while ((elem = nextElement(&iter)) != NULL) {
		Path *tempPath = (Path *)elem;
		if (strcasecmp(data, tempPath->data) == 0) {
			sum++;
		}
	}
	freeList(pathList);
	return sum;
}
int numGroupsWithLen(SVGimage* img, int len) {
	if (img == NULL || len < 0) {
		return 0;
	}
	
	int sum = 0;
	void *elem;
	List *groupList = getGroups(img);
	ListIterator iter = createIterator(groupList);
	while ((elem = nextElement(&iter)) != NULL) {
		Group *tempGroup = (Group *)elem;
		int length = getLength(tempGroup->rectangles) + getLength(tempGroup->circles) + getLength(tempGroup->paths) + getLength(tempGroup->groups);
		if (len == length) {
			sum++;
		}
	}
	freeList(groupList);
	return sum;
}

int numAttr(SVGimage* img) {
	if (img == NULL) {
		return 0;
	}
	int sum = 0;
	void *elem;
	ListIterator iter;
	List *list;
	
	//Counting in SVG
	sum += getLength(img->otherAttributes);
	
	//Counting in Rects
	list = getRects(img);
	iter = createIterator(list);
	while ((elem = nextElement(&iter)) != NULL) {
		Rectangle *tempRect = (Rectangle *)elem;
		sum += getLength(tempRect->otherAttributes);
	}
	freeList(list);
	
	//Counting in Circle
	list = getCircles(img);
	iter = createIterator(list);
	while ((elem = nextElement(&iter)) != NULL) {
		Circle *tempCircle = (Circle *)elem;
		sum += getLength(tempCircle->otherAttributes);
	}
	freeList(list);
	
	//Counting in Paths
	list = getPaths(img);
	iter = createIterator(list);
	while ((elem = nextElement(&iter)) != NULL) {
		Path *tempPath = (Path *)elem;
		sum += getLength(tempPath->otherAttributes);
	}
	freeList(list);
	
	//Counting in Groups
	list = getGroups(img);
	iter = createIterator(list);
	while ((elem = nextElement(&iter)) != NULL) {
		Group *tempGroup = (Group *)elem;
		sum += getLength(tempGroup->otherAttributes);
	}
	freeList(list);
	
	
	return sum;
}

/* ********** List helper functions ********** */

//Attributes
void deleteAttribute (void* data) {
	if (data != NULL) {
		free(((Attribute *)data)->name);
		free(((Attribute *)data)->value);
		free(data);
	}
}
char* attributeToString (void* data) {
	char *str = malloc(strlen(((Attribute *)data)->name) + strlen(((Attribute *)data)->value) + 16);
	if (data != NULL) {
		sprintf(str, "%s: %s ", ((Attribute *)data)->name, ((Attribute *)data)->value);
	}
	
	return str;
}
int compareAttributes (const void *first, const void *second) {
	return 0;
}

//Group
void deleteGroup(void* data) {
	if (data == NULL) {
		return;
	}
	freeList(((Group *)data)->rectangles);
	freeList(((Group *)data)->circles);
	freeList(((Group *)data)->paths);
	freeList(((Group *)data)->groups);
	freeList(((Group *)data)->otherAttributes);
	free(data);
}
char* groupToString(void* data) {
	char *string = malloc(10);
	char *buffer;
	ListIterator iter;
	void *elem;
	
	strcpy(string, "Group:\n");
	
	//Adding Rectangles to string
	iter = createIterator(((Group *)data)->rectangles);
	while ((elem = nextElement(&iter)) != NULL) {
		Rectangle *tempRect = (Rectangle *)elem;
		buffer = ((Group *)data)->rectangles->printData(tempRect);
		string = realloc(string, strlen(string) + strlen(buffer) + 16);
		strcat(string, buffer);
		free(buffer);
	}
	//if (getLength(((Group *)data)->rectangles) == 0) {

	//Adding Circles to string
	iter = createIterator(((Group *)data)->circles);
	while ((elem = nextElement(&iter)) != NULL) {
		Circle *tempCircle = (Circle *)elem;
		buffer = ((Group *)data)->circles->printData(tempCircle);
		string = realloc(string, strlen(string) + strlen(buffer) + 16);
		strcat(string, buffer);
		free(buffer);
	}

	//Adding Paths to string
	iter = createIterator(((Group *)data)->paths);
	while ((elem = nextElement(&iter)) != NULL) {
		Path *tempPath = (Path *)elem;
		buffer = ((Group *)data)->paths->printData(tempPath);
		string = realloc(string, strlen(string) + strlen(buffer) + 16);
		strcat(string, buffer);
		free(buffer);
	}
	
	//Adding groups to string
	iter = createIterator(((Group *)data)->groups);
	while ((elem = nextElement(&iter)) != NULL) {
		Group *tempGroup = (Group *)elem;
		buffer = ((Group *)data)->groups->printData(tempGroup);
		string = realloc(string, strlen(string) + strlen(buffer) + 16);
		strcat(string, buffer);
		free(buffer);
	}
	
	//Adding other attributes to string
	iter = createIterator(((Group *)data)->otherAttributes);
	if (getLength(((Group *)data)->otherAttributes) > 0) {
		string = realloc(string, strlen(string) + 50);
		strcat(string, "Other Attributes:\n"); //Formatting
	}
	while ((elem = nextElement(&iter)) != NULL){
		Attribute *tempAttr = (Attribute *)elem;
		buffer = ((Group *)data)->otherAttributes->printData(tempAttr);
		string = realloc(string, strlen(string) + strlen(buffer) + 16);
		strcat(string, "\t"); //Formatting
		strcat(string, buffer);
		strcat(string, "\n"); //Formatting
		free(buffer);
	}

	return string;
}
int compareGroups(const void *first, const void *second) {
	return 0;
}

//Rectangle
void deleteRectangle(void* data) {
	if (data != NULL) {
		freeList(((Rectangle *)data)->otherAttributes);
		free(data);
	}
}
char* rectangleToString(void* data) {
	char *str;
	if (data != NULL) {
		str = malloc(sizeof(float) * 4 + 1000);
		sprintf(str, "Rectangle:\n\tx: %f, y: %f, Width: %f, Height: %f, " , ((Rectangle*)data)->x, ((Rectangle*)data)->y, ((Rectangle*)data)->width, ((Rectangle*)data)->height);
		if (strcasecmp(((Rectangle*)data)->units, "") != 0) {
			char buffer[100];
			sprintf(buffer, "Units: %s \n\t", ((Rectangle*)data)->units);
			str = realloc(str, strlen(str) + strlen(buffer) + 16);
			strcat(str, buffer);
		} else {
			strcat(str, "Units: None \n\t");
		}
		void* elem;
		ListIterator iter = createIterator(((Rectangle*)data)->otherAttributes);
		while ((elem = nextElement(&iter)) != NULL){
			Attribute *tempAttr = (Attribute *)elem;
			char* attributeString = ((Rectangle*)data)->otherAttributes->printData(tempAttr);
			str = realloc(str, strlen(str) + strlen(attributeString) + 16);
			strcat(str, attributeString);
			free(attributeString);
		}
		strcat(str, "\n");
	}
	return str;
}
int compareRectangles(const void *first, const void *second) {
	return 0;
}

//Circle
void deleteCircle(void* data) {
	if (data != NULL) {
		freeList(((Circle *)data)->otherAttributes);
		free(data);
	}
}
char* circleToString(void* data) {
	char *str;
	if (data != NULL) {
		str = malloc(sizeof(float) * 3 + 100);
		sprintf(str, "Circle:\n\tcx: %f, cy: %f, r: %f, " , ((Circle *)data)->cx, ((Circle *)data)->cy, ((Circle*)data)->r);
		if (strcasecmp(((Circle *)data)->units, "") != 0) {
			char buffer[100];
			sprintf(buffer, "Units: %s \n\t", ((Circle*)data)->units);
			strcat(str, buffer);
		} else {
			strcat(str, "Units: None \n\t");
		}
		void* elem;
		ListIterator iter = createIterator(((Circle *)data)->otherAttributes);
		while ((elem = nextElement(&iter)) != NULL){
			Attribute *tempAttr = (Attribute *)elem;
			char* attributeString = ((Circle *)data)->otherAttributes->printData(tempAttr);
			str = realloc(str, strlen(str) + strlen(attributeString) + 16);
			strcat(str, attributeString);
			free(attributeString);
		}
		strcat(str, "\n");
	}
	
	return str;
}
int compareCircles(const void *first, const void *second) {
	return 0;
}

//Path
void deletePath(void* data) {
	if (data != NULL) {
		freeList(((Path *)data)->otherAttributes);
		free(((Path *)data)->data);
		free(data);
	}
}
char* pathToString(void* data) {
	char *str;
	if (data != NULL) {
		str = malloc(strlen(((Path *)data)->data) + 16);
		sprintf(str, "Path:\n\td: %s\n\t", ((Path *)data)->data);
		
		//Looping through Attributes
		void* elem;
		ListIterator iter = createIterator(((Path *)data)->otherAttributes);
		while ((elem = nextElement(&iter)) != NULL){
			Attribute *tempAttr = (Attribute *)elem;
			char* attributeString = ((Path *)data)->otherAttributes->printData(tempAttr);
			str = realloc(str, strlen(str) + strlen(attributeString) + 16);
			strcat(str, attributeString);
			strcat(str, " ");
			free(attributeString);
		}
		strcat(str, "\n");
	}
	
	return str;
}
int comparePaths(const void *first, const void *second) {
	return 0;
}

/* A2 Stuff*/

/* Code provided from http://knol2share.blogspot.com/2009/05/validate-xml-against-xsd-in-c.html used to validate svg file with schema file used. */
/** Function to create an SVG object based on the contents of an SVG file.
 * This function must validate the XML tree generated by libxml against a SVG schema file
 * before attempting to traverse the tree and create an SVGimage struct
 *@pre File name cannot be an empty string or NULL.
       File represented by this name must exist and must be readable.
       Schema file name is not NULL/empty, and represents a valid schema file
 *@post Either:
        A valid SVGimage has been created and its address was returned
		or 
		An error occurred, or SVG file was invalid, and NULL was returned
 *@return the pointer to the new struct or NULL
 *@param fileName - a string containing the name of the SVG file
**/
SVGimage* createValidSVGimage(char* fileName, char* schemaFile) {
	SVGimage *image = createSVGimage(fileName);

	if (image == NULL) {
		return NULL;
	}
	if (validateSVGimage(image, schemaFile) == true) { //SVG is valid
		return image;
	} else {
		deleteSVGimage(image);
		return NULL;
	}
}

/** Function to writing a SVGimage into a file in SVG format.
 *@pre
    SVGimage object exists, is valid, and and is not NULL.
    fileName is not NULL, has the correct extension
 *@post SVGimage has not been modified in any way, and a file representing the
    SVGimage contents in SVG format has been created
 *@return a boolean value indicating success or failure of the write
 *@param
    doc - a pointer to a SVGimage struct
 	fileName - the name of the output file
 **/
bool writeSVGimage(SVGimage* image, char* fileName) {
	xmlDocPtr doc = imageToTree(image);
	if (doc != NULL) {
		if (xmlSaveFormatFileEnc(fileName, doc, NULL, 1) != -1) {
			xmlFreeDoc(doc);
			xmlCleanupParser();
			return true;
		} else {
			xmlFreeDoc(doc);
			xmlCleanupParser();
			return false;
		}
	} else {
		xmlCleanupParser();
		return false;
	}
}

/** Function to validating an existing a SVGimage object against a SVG schema file
 *@pre 
    SVGimage object exists and is not NULL
    schema file name is not NULL/empty, and represents a valid schema file
 *@post SVGimage has not been modified in any way
 *@return the boolean aud indicating whether the SVGimage is valid
 *@param obj - a pointer to a GPXSVGimagedoc struct
 *@param obj - the name iof a schema file
 **/
bool validateSVGimage(SVGimage* image, char* schemaFile) {
	if (validateSVGwithSchema(image, schemaFile) == true && validateSVGwithHeader(image) == true) {
		return true;
	} else {
		return false;
	}
}

/* ***** A2 Helper Functions ***** */

/* A group of functions which converts SVGimage struct to xmlDocPtr */
xmlDocPtr imageToTree(SVGimage *image) {
	if (image == NULL) {
		return NULL;
	}
	
	xmlDocPtr doc = NULL; //Document pointer
    xmlNodePtr root_node = NULL;
    
    //Creates a new document, and root <svg> node
    doc = xmlNewDoc(BAD_CAST "1.0");
    root_node = xmlNewNode(NULL, BAD_CAST "svg");
    xmlDocSetRootElement(doc, root_node);
    
    //xmlCreateIntSubset(doc, BAD_CAST "svg", BAD_CAST "Monkey", BAD_CAST image->namespace);
    
	svgToTree(root_node, image); //svg
    rectToTree(root_node, image->rectangles); //rect
    circleToTree(root_node, image->circles); //circle
    pathToTree(root_node, image->paths); //path
    groupToTree(root_node, image->groups); //g

    //xmlCleanupParser();
	xmlMemoryDump();
    
	return doc;
}
void svgToTree(xmlNode *node, SVGimage *image) {
    //Other Attributes in <svg>
    if (image->otherAttributes == NULL) {
		return;
	}
	
	Attribute *elem;
	ListIterator iter = createIterator(image->otherAttributes);
	while ((elem = nextElement(&iter)) != NULL){
		xmlNewProp(node, BAD_CAST elem->name, BAD_CAST elem->value);
	}
	
	xmlSetNs(node, xmlNewNs(node, BAD_CAST image->namespace, NULL)); //namespace but theres some magic
	
	if (strcmp(image->title, "") != 0) {
		xmlNewTextChild(node, NULL, BAD_CAST "title", BAD_CAST image->title);
	}
	if (strcmp(image->description, "") != 0) {
		xmlNewTextChild(node, NULL, BAD_CAST "desc", BAD_CAST image->description);
	}
}
void rectToTree(xmlNodePtr parent_node, List *rectList) {
	if (rectList == NULL) {
		return;
	}
	xmlNodePtr rect_Node = NULL;
	
	//Looping through all rects
	Rectangle *tempRect;
	ListIterator iter = createIterator(rectList);
	while ((tempRect = nextElement(&iter)) != NULL){
		rect_Node = xmlNewChild(parent_node, NULL, BAD_CAST "rect", NULL); //Setting node as child
		
		char string[5000];
		//x
		sprintf(string, "%.5f%s", tempRect->x, tempRect->units);
		xmlNewProp(rect_Node, BAD_CAST "x", BAD_CAST string);
		//y
		sprintf(string, "%.5f%s", tempRect->y, tempRect->units);
		xmlNewProp(rect_Node, BAD_CAST "y", BAD_CAST string);
		//width
		sprintf(string, "%.5f%s", tempRect->width, tempRect->units);
		xmlNewProp(rect_Node, BAD_CAST "width", BAD_CAST string);
		//height
		sprintf(string, "%.5f%s", tempRect->height, tempRect->units);
		xmlNewProp(rect_Node, BAD_CAST "height", BAD_CAST string);
		
		//Looping through other attributes in rect
		if (tempRect->otherAttributes != NULL) {
			Attribute *tempAttr;
			ListIterator iter = createIterator(tempRect->otherAttributes);
			while ((tempAttr = nextElement(&iter)) != NULL){
				xmlNewProp(rect_Node, BAD_CAST tempAttr->name, BAD_CAST tempAttr->value);
			}
		}
	}
}
void circleToTree(xmlNodePtr parent_node, List *circleList) {
	if (circleList == NULL) {
		return;
	}
	xmlNodePtr circle_Node = NULL;
	
	//Looping through all circles
	Circle *tempCircle;
	ListIterator iter = createIterator(circleList);
	while ((tempCircle = nextElement(&iter)) != NULL){
		circle_Node = xmlNewChild(parent_node, NULL, BAD_CAST "circle", NULL); //Setting node as child
		
		char string[5000];
		//cx
		sprintf(string, "%.5f%s", tempCircle->cx, tempCircle->units);
		xmlNewProp(circle_Node, BAD_CAST "cx", BAD_CAST string);
		//cy
		sprintf(string, "%.5f%s", tempCircle->cy, tempCircle->units);
		xmlNewProp(circle_Node, BAD_CAST "cy", BAD_CAST string);
		//r
		sprintf(string, "%.5f%s", tempCircle->r, tempCircle->units);
		xmlNewProp(circle_Node, BAD_CAST "r", BAD_CAST string);
		
		//Looping through other attributes in circle
		if (tempCircle->otherAttributes != NULL) {
			Attribute *tempAttr;
			ListIterator iter = createIterator(tempCircle->otherAttributes);
			while ((tempAttr = nextElement(&iter)) != NULL){
				xmlNewProp(circle_Node, BAD_CAST tempAttr->name, BAD_CAST tempAttr->value);
			}
		}
	}
}
void pathToTree(xmlNodePtr parent_node, List *pathList) {
	if (pathList == NULL) {
		return;
	}
	
	xmlNodePtr path_Node = NULL;
	
	//Looping through all paths
	Path *tempPath;
	ListIterator iter = createIterator(pathList);
	while ((tempPath = nextElement(&iter)) != NULL){
		path_Node = xmlNewChild(parent_node, NULL, BAD_CAST "path", NULL); //Setting node as child
		
		char string[50000];
		//data
		sprintf(string, "%s", tempPath->data);
		xmlNewProp(path_Node, BAD_CAST "d", BAD_CAST string);
		
		//Looping through other attributes in path
		if (tempPath->otherAttributes != NULL) {
			Attribute *tempAttr;
			ListIterator iter = createIterator(tempPath->otherAttributes);
			while ((tempAttr = nextElement(&iter)) != NULL){
				xmlNewProp(path_Node, BAD_CAST tempAttr->name, BAD_CAST tempAttr->value);
			}
		}
	}
}
void groupToTree(xmlNodePtr parent_node, List *groupList) {
	if (groupList == NULL) {
		return;
	}
	
	xmlNodePtr group_Node = NULL;
	
	//Looping through all groups
	Group *tempGroup;
	ListIterator iter = createIterator(groupList);
	while ((tempGroup = nextElement(&iter)) != NULL){
		group_Node = xmlNewChild(parent_node, NULL, BAD_CAST "g", NULL); //Setting node as child
		
		//Looping through other attributes in group
		if (tempGroup->otherAttributes != NULL) {
			Attribute *tempAttr;
			ListIterator iter = createIterator(tempGroup->otherAttributes);
			while ((tempAttr = nextElement(&iter)) != NULL){
				xmlNewProp(group_Node, BAD_CAST tempAttr->name, BAD_CAST tempAttr->value);
			}
		}
		
		//Adding other shapes as children
		rectToTree(group_Node, tempGroup->rectangles);
		circleToTree(group_Node, tempGroup->circles);
		pathToTree(group_Node, tempGroup->paths);
		
		//Adding groups recursively
		groupToTree(group_Node, tempGroup->groups);
	}
}

/* Validates an SVG image with a schema file by using the tree functions above */
bool validateSVGwithSchema(SVGimage *image, char* schemaFile) {
	xmlSchemaParserCtxtPtr ctxt = xmlSchemaNewParserCtxt(schemaFile);
	xmlSchemaPtr schema = NULL;
	schema = xmlSchemaParse(ctxt);
	xmlSchemaFreeParserCtxt(ctxt);

	bool valid = true; //true == Valid, false == Invalid
	xmlDocPtr doc = imageToTree(image);
	if (doc == NULL) { //image doesnt exist
		valid = false;
	} else {
		xmlSchemaValidCtxtPtr ctxt = xmlSchemaNewValidCtxt(schema);
		int ret = xmlSchemaValidateDoc(ctxt, doc);
		if (ret == 0) { //Valid SVG
			valid = true;
		} else if (ret > 0) { //Invalid SVG
			valid = false;
		} else { //Invalid SVG, internal Error
			valid = false;
		}
		xmlSchemaFreeValidCtxt(ctxt);
		xmlFreeDoc(doc); //Careful that this doesn't overlap with the doc created in createSVGimage()
	}
	
	//Freeing
	if (schema != NULL) {
		xmlSchemaFree(schema);
	}
	xmlSchemaCleanupTypes();
	xmlCleanupParser();
	xmlMemoryDump();
	
	return valid;
}

/* Validates an SVG image with the header definitions */
bool validateSVGwithHeader(SVGimage *image) {
	List *list = NULL;
	
	//Checking namespace
	if (strlen(image->namespace) < 1) {
		return false;
	}
	
	//Checking rectangles
	list = getRects(image);
	if (validateRects(list) == false) {
		freeList(list);
		return false;
	}
	freeList(list);
	
	//Checking circles
	list = getCircles(image);
	if (validateCircles(list) == false) {
		freeList(list);
		return false;
	}
	freeList(list);
	
	//Checking paths
	list = getPaths(image);
	if (validatePaths(list) == false) {
		freeList(list);
		return false;
	}
	freeList(list);
	
	//Checking group
	list = getGroups(image);
	if (validateGroups(list) == false) {
		freeList(list);
		return false;
	}
	freeList(list);
	
	return true;
}
bool validateRects(List *rectList) {
	if (rectList == NULL) {
		return false;
	}
	
	Rectangle *tempRect;
	ListIterator iter = createIterator(rectList);
	while ((tempRect = nextElement(&iter)) != NULL) {
		if (tempRect->width < 0) {
			return false;
		}
		if (tempRect->height < 0) {
			return false;
		}
		if (validateAttributes(tempRect->otherAttributes) == false) {
			return false;
		}
	}
	
	return true;
}
bool validateCircles(List *circleList) {
	if (circleList == NULL) {
		return false;
	}
	
	Circle *tempCircle;
	ListIterator iter = createIterator(circleList);
	while ((tempCircle = nextElement(&iter)) != NULL) {
		if (tempCircle->r < 0) {
			return false;
		}
		if (validateAttributes(tempCircle->otherAttributes) == false) {
			return false;
		}
	}
	
	return true;
}
bool validatePaths(List *pathList) {
	if (pathList == NULL) {
		return false;
	}
	
	Path *tempPath;
	ListIterator iter = createIterator(pathList);
	while ((tempPath = nextElement(&iter)) != NULL) {
		if (tempPath->data == NULL) {
			return false;
		}
		if (validateAttributes(tempPath->otherAttributes) == false) {
			return false;
		}
	}
	
	return true;
}
bool validateGroups(List *groupList) {
	if (groupList == NULL) {
		return false;
	}
	
	Group *tempGroup;
	ListIterator iter = createIterator(groupList);
	while ((tempGroup = nextElement(&iter)) != NULL) {
		if (validateAttributes(tempGroup->otherAttributes) == false) {
			return false;
		}
	}
	
	return true;
}
bool validateAttributes(List *attrList) {
	if (attrList == NULL) {
		return false;
	}
	
	Attribute *tempAttr;
	ListIterator iter = createIterator(attrList);
	while ((tempAttr = nextElement(&iter)) != NULL) {
		if (tempAttr->name == NULL) {
			return false;
		}
		if (tempAttr->value == NULL) {
			return false;
		}
	}
	
	return true;
}

/* Helper functions for setAttribute() */
void setAttribute(SVGimage* image, elementType elemType, int elemIndex, Attribute* newAttribute) {
	if (image == NULL || !(elemType == 0 || elemType == 1 || elemType == 2 || elemType == 3) || newAttribute == NULL || newAttribute->name == NULL || newAttribute->value == NULL) {
		return;
	}
	
	if (elemType == SVG_IMAGE) { //svg
		setSVG(image, elemType, elemIndex, newAttribute);
	} else if (elemType == RECT) { //rectangle
		setRect(image, elemType, elemIndex, newAttribute);
	} else if (elemType == CIRC) { //circle
		setCircle(image, elemType, elemIndex, newAttribute);
	} else if (elemType == PATH) { //path
		setPath(image, elemType, elemIndex, newAttribute);
	} else if (elemType == GROUP) { //group
		setGroup(image, elemType, elemIndex, newAttribute);
	}
}
void setSVG(SVGimage* image, elementType elemType, int elemIndex, Attribute* newAttribute) {
	if (strcmp(newAttribute->name, "namespace") == 0) {
		strncpy(image->namespace, newAttribute->value, 255);
		deleteAttribute(newAttribute);
	}
	else if (strcmp(newAttribute->name, "title") == 0) {
		strncpy(image->title, newAttribute->value, 255);
		deleteAttribute(newAttribute);
	}
	else if (strcmp(newAttribute->name, "description") == 0) {
		strncpy(image->description, newAttribute->value, 255);
		deleteAttribute(newAttribute);
	}
	else {
		int found = 0; //0 = not found, 1 = found
		Attribute *attr;
		ListIterator iter = createIterator(image->otherAttributes);
		while ((attr = nextElement(&iter)) != NULL) {
			if (strcmp(attr->name, newAttribute->name) == 0) { //Found attribute with same name, just update
				attr->value = realloc(attr->value, strlen(newAttribute->value) + 4);
				strcpy(attr->value, newAttribute->value);
				deleteAttribute(newAttribute);
				found = 1;
				break;
			}
		}
		if (found == 0) { //Attribute doesnt exist, lets add it
			insertBack(image->otherAttributes, newAttribute);
		}
	}
}
void setRect(SVGimage* image, elementType elemType, int elemIndex, Attribute* newAttribute) {
	if (elemIndex < 0 || elemIndex >= getLength(image->rectangles)) {
		return;
	}
	
	int i = 0;
	Rectangle *rect;
	ListIterator iter = createIterator(image->rectangles);
	while ((rect = nextElement(&iter)) != NULL) {
		if (elemIndex == i) { //Rectangle located!
			break;
		}
		++i;
	}
	
	if (strcmp(newAttribute->name, "x") == 0) {
		rect->x = (float)atof(newAttribute->value);
		deleteAttribute(newAttribute);
	} 
	else if (strcmp(newAttribute->name, "y") == 0) {
		rect->y = (float)atof(newAttribute->value);
		deleteAttribute(newAttribute);
	} 
	else if (strcmp(newAttribute->name, "width") == 0) {
		if ((float)atof(newAttribute->value) < 0) {
			return; //Do nothing if parameter invalid
		}
		rect->width = (float)atof(newAttribute->value);
		deleteAttribute(newAttribute);
	}
	else if (strcmp(newAttribute->name, "height") == 0) {
		if ((float)atof(newAttribute->value) < 0) {
			return; //Do nothing if parameter invalid
		}
		rect->height = (float)atof(newAttribute->value);
		deleteAttribute(newAttribute);
	}
	else if (strcmp(newAttribute->name, "units") == 0) {
		strncpy(rect->units, newAttribute->value, 49);
		deleteAttribute(newAttribute);
	}
	else { //Other attribute
		int found = 0; //0 = not found, 1 = found
		Attribute *attr;
		iter = createIterator(rect->otherAttributes);
		while ((attr = nextElement(&iter)) != NULL) {
			if (strcmp(attr->name, newAttribute->name) == 0) { //Found attribute with same name, just update
				attr->value = realloc(attr->value, strlen(newAttribute->value) + 4);
				strcpy(attr->value, newAttribute->value);
				deleteAttribute(newAttribute);
				found = 1;
				break;
			}
		}
		if (found == 0) { //Attribute doesnt exist, lets add it
			insertBack(rect->otherAttributes, newAttribute);
			
		}
	}
}
void setCircle(SVGimage* image, elementType elemType, int elemIndex, Attribute* newAttribute) {
	if (elemIndex < 0 || elemIndex >= getLength(image->circles)) {
		return;
	}
	
	//Finding the correct circle using elemIndex
	int i = 0;
	Circle *circle;
	ListIterator iter = createIterator(image->circles);
	while ((circle = nextElement(&iter)) != NULL) {
		if (elemIndex == i) { //Circle located!
			break;
		}
		++i;
	}
	
	if (strcmp(newAttribute->name, "cx") == 0) {
		circle->cx = (float)atof(newAttribute->value);
		deleteAttribute(newAttribute);
	} 
	else if (strcmp(newAttribute->name, "cy") == 0) {
		circle->cy = (float)atof(newAttribute->value);
		deleteAttribute(newAttribute);
	} 
	else if (strcmp(newAttribute->name, "r") == 0) {
		if ((float)atof(newAttribute->value) < 0) {
			return; //Do nothing if parameter invalid
		}
		circle->r = (float)atof(newAttribute->value);
		deleteAttribute(newAttribute);
	}
	else if (strcmp(newAttribute->name, "units") == 0) {
		strncpy(circle->units, newAttribute->value, 49);
		deleteAttribute(newAttribute);
	}
	else { //Other attribute
		int found = 0; //0 = not found, 1 = found
		Attribute *attr;
		iter = createIterator(circle->otherAttributes);
		while ((attr = nextElement(&iter)) != NULL) {
			if (strcmp(attr->name, newAttribute->name) == 0) { //Found attribute with same name, just update
				attr->value = realloc(attr->value, strlen(newAttribute->value) + 4);
				strcpy(attr->value, newAttribute->value);
				deleteAttribute(newAttribute);
				found = 1;
				break;
			}
		}
		if (found == 0) { //Attribute doesnt exist, lets add it
			insertBack(circle->otherAttributes, newAttribute);
			
		}
	}
}
void setPath(SVGimage* image, elementType elemType, int elemIndex, Attribute* newAttribute) {
	if (elemIndex < 0 || elemIndex >= getLength(image->paths)) {
		return;
	}
	
	//Finding the correct path using elemIndex
	int i = 0;
	Path *path;
	ListIterator iter = createIterator(image->paths);
	while ((path = nextElement(&iter)) != NULL) {
		if (elemIndex == i) { //Path located!
			break;
		}
		++i;
	}
	
	if (strcmp(newAttribute->name, "d") == 0) {
		path->data = realloc(path->data, strlen(newAttribute->value) + 4);
		strcpy(path->data, newAttribute->value);
		deleteAttribute(newAttribute);
	} 
	else { //Other attribute
		int found = 0; //0 = not found, 1 = found
		Attribute *attr;
		iter = createIterator(path->otherAttributes);
		while ((attr = nextElement(&iter)) != NULL) {
			if (strcmp(attr->name, newAttribute->name) == 0) { //Found attribute with same name, just update
				attr->value = realloc(attr->value, strlen(newAttribute->value) + 4);
				strcpy(attr->value, newAttribute->value);
				deleteAttribute(newAttribute);
				found = 1;
				break;
			}
		}
		if (found == 0) { //Attribute doesnt exist, lets add it
			insertBack(path->otherAttributes, newAttribute);
			
		}
	}
}
void setGroup(SVGimage* image, elementType elemType, int elemIndex, Attribute* newAttribute) {
	if (elemIndex < 0 || elemIndex >= getLength(image->groups)) {
		return;
	}
	
	//Finding the correct group using elemIndex
	int i = 0;
	Group *group;
	ListIterator iter = createIterator(image->groups);
	while ((group = nextElement(&iter)) != NULL) {
		if (elemIndex == i) { //Group located!
			break;
		}
		++i;
	}
	
	//Can only be other attribute
	int found = 0; //0 = not found, 1 = found
	Attribute *attr;
	iter = createIterator(group->otherAttributes);
	while ((attr = nextElement(&iter)) != NULL) {
		if (strcmp(attr->name, newAttribute->name) == 0) { //Found attribute with same name, just update
			attr->value = realloc(attr->value, strlen(newAttribute->value) + 4);
			strcpy(attr->value, newAttribute->value);
			deleteAttribute(newAttribute);
			found = 1;
			break;
		}
	}
	if (found == 0) { //Attribute doesnt exist, lets add it
		insertBack(group->otherAttributes, newAttribute);
	}
}

void addComponent(SVGimage* image, elementType elemType, void* newComponent) {
	if (image == NULL || !(elemType == RECT || elemType == CIRC || elemType == PATH) || newComponent == NULL || elemType == SVG_IMAGE || elemType == GROUP) {
		return;
	}
	
	if (elemType == RECT) { //rectangle
		if (((Rectangle*)newComponent)->otherAttributes == NULL) {
			return;
		}
		insertBack(image->rectangles, newComponent);
	} else if (elemType == CIRC) { //circle
		if (((Circle*)newComponent)->otherAttributes == NULL) {
			return;
		}
		insertBack(image->circles, newComponent);
	} else if (elemType == PATH) { //path
		if (((Path*)newComponent)->data == NULL || ((Path*)newComponent)->otherAttributes == NULL) {
			return;
		}
		insertBack(image->paths, newComponent);
	}
}

/* Set of functions that convert the specified parameter to JSON string for web side */
char* attrToJSON(const Attribute *a) {
	char *buffer = malloc(10);
	strcpy(buffer, "{}");
	if (a == NULL || a->name == NULL || a->value == NULL) {
		return buffer;
	}
	
	buffer = realloc(buffer, 50 + strlen(a->name) + strlen(a->value));
	sprintf(buffer, "{\"name\":\"%s\",\"value\":\"%s\"}", a->name, a->value);
	
	return buffer;
}
char* rectToJSON(const Rectangle *r) {
	char *buffer = malloc(10);
	strcpy(buffer, "{}");
	if (r == NULL || r->units == NULL || r->otherAttributes == NULL) {
		return buffer;
	}
	
	buffer = realloc(buffer, 50 + sizeof(float) * 4 + strlen(r->units));
	sprintf(buffer, "{\"x\":%.2f,\"y\":%.2f,\"w\":%.2f,\"h\":%.2f,\"numAttr\":%d,\"units\":\"%s\"}", r->x, r->y, r->width, r->height, getLength(r->otherAttributes), r->units); 
	
	return buffer;
}
char* circleToJSON(const Circle *c) {
	char *buffer = malloc(10);
	strcpy(buffer, "{}");
	if (c == NULL || c->units == NULL || c->otherAttributes == NULL) {
		return buffer;
	}
	
	buffer = realloc(buffer, 50 + sizeof(float) * 3 + strlen(c->units));
	sprintf(buffer, "{\"cx\":%.2f,\"cy\":%.2f,\"r\":%.2f,\"numAttr\":%d,\"units\":\"%s\"}", c->cx, c->cy, c->r, getLength(c->otherAttributes), c->units); 
	
	return buffer;
}
char* pathToJSON(const Path *p) {
	char *buffer = malloc(10);
	strcpy(buffer, "{}");
	if (p == NULL || p->data == NULL || p->otherAttributes == NULL) {
		return buffer;
	}
	
	buffer = realloc(buffer, 50 + strlen(p->data));
	
	char *buffer2 = malloc(100);
	snprintf(buffer2, 65, "%s", p->data);
	sprintf(buffer, "{\"d\":\"%s\",\"numAttr\":%d}", buffer2, getLength(p->otherAttributes)); 
	
	return buffer;
}
char* groupToJSON(const Group *g) {
	char *buffer = malloc(50);
	strcpy(buffer, "{}");
	if (g == NULL || g->rectangles == NULL || g->circles == NULL || g->paths == NULL || g->groups == NULL || g->otherAttributes == NULL) {
		return buffer;
	}
	
	int totalChildren = getLength(g->rectangles) + getLength(g->circles) + getLength(g->paths) + getLength(g->groups);
	sprintf(buffer, "{\"children\":%d,\"numAttr\":%d}", totalChildren, getLength(g->otherAttributes)); 
	
	return buffer;
}

char* SVGtoJSON(const SVGimage* image) {
	char *buffer = malloc(150);
	strcpy(buffer, "{}");
	if (image == NULL || image->rectangles == NULL || image->circles == NULL || image->paths == NULL || image->groups == NULL || image->otherAttributes == NULL) {
		return buffer;
	}
	List *list;
	
	//Number of rectangles
	list = getRects((SVGimage*)image);
	int numRects = getLength(list);
	freeList(list);
	
	//Number of circles
	list = getCircles((SVGimage*)image);
	int numCircles = getLength(list);;
	freeList(list);
	
	//Number of paths
	list = getPaths((SVGimage*)image);
	int numPaths = getLength(list);;
	freeList(list);
	
	//Number of groups
	list = getGroups((SVGimage*)image);
	int numGroups = getLength(list);;
	freeList(list);
	
	sprintf(buffer, "{\"numRect\":%d,\"numCirc\":%d,\"numPaths\":%d,\"numGroups\":%d}", numRects, numCircles, numPaths, numGroups); 
	
	return buffer;
}

char* attrListToJSON(const List *list) {
	//Checking if parameter is valid
	char *buffer = malloc(50);
	strcpy(buffer, "[]");
	if (list == NULL || getLength((List*)list) == 0) {
		return buffer;
	}
	
	//Creating string
	strcpy(buffer, "[");
	Attribute *a;
	ListIterator iter = createIterator((List*)list);
	while ((a = nextElement(&iter)) != NULL) {
		char *tempStr = attrToJSON(a);
		buffer = realloc(buffer, strlen(buffer) + strlen(tempStr) + 10);
		strcat(buffer, tempStr);
		free(tempStr); //String allocated by attrToJSON is dynamic
		strcat(buffer, ",");
	}
	buffer[strlen(buffer) - 1] = '\0';
	strcat(buffer, "]");
	
	return buffer;
}
char* rectListToJSON(const List *list) {
	//Checking if parameter is valid
	char *buffer = malloc(50);
	strcpy(buffer, "[]");
	if (list == NULL || getLength((List*)list) == 0) {
		return buffer;
	}
	
	//Creating string
	strcpy(buffer, "[");
	Rectangle *r;
	ListIterator iter = createIterator((List*)list);
	while ((r = nextElement(&iter)) != NULL) {
		char *tempStr = rectToJSON(r);
		buffer = realloc(buffer, strlen(buffer) + strlen(tempStr) + 10);
		strcat(buffer, tempStr);
		free(tempStr); //String allocated by rectToJSON is dynamic
		strcat(buffer, ",");
	}
	buffer[strlen(buffer) - 1] = '\0';
	strcat(buffer, "]");
	
	return buffer;
}
char* circListToJSON(const List *list) {
	//Checking if parameter is valid
	char *buffer = malloc(50);
	strcpy(buffer, "[]");
	if (list == NULL || getLength((List*)list) == 0) {
		return buffer;
	}
	
	//Creating string
	strcpy(buffer, "[");
	Circle *c;
	ListIterator iter = createIterator((List*)list);
	while ((c = nextElement(&iter)) != NULL) {
		char *tempStr = circleToJSON(c);
		buffer = realloc(buffer, strlen(buffer) + strlen(tempStr) + 10);
		strcat(buffer, tempStr);
		free(tempStr); //String allocated by circleToJSON is dynamic
		strcat(buffer, ",");
	}
	buffer[strlen(buffer) - 1] = '\0';
	strcat(buffer, "]");
	
	return buffer;
}
char* pathListToJSON(const List *list) {
	//Checking if parameter is valid
	char *buffer = malloc(50);
	strcpy(buffer, "[]");
	if (list == NULL || getLength((List*)list) == 0) {
		return buffer;
	}
	
	//Creating string
	strcpy(buffer, "[");
	Path *p;
	ListIterator iter = createIterator((List*)list);
	while ((p = nextElement(&iter)) != NULL) {
		char *tempStr = pathToJSON(p);
		buffer = realloc(buffer, strlen(buffer) + strlen(tempStr) + 10);
		strcat(buffer, tempStr);
		free(tempStr); //String allocated by pathToJSON is dynamic
		strcat(buffer, ",");
	}
	buffer[strlen(buffer) - 1] = '\0';
	strcat(buffer, "]");
	
	return buffer;
}
char* groupListToJSON(const List *list) {
	//Checking if parameter is valid
	char *buffer = malloc(50);
	strcpy(buffer, "[]");
	if (list == NULL || getLength((List*)list) == 0) {
		return buffer;
	}
	
	//Creating string
	strcpy(buffer, "[");
	Group *g;
	ListIterator iter = createIterator((List*)list);
	while ((g = nextElement(&iter)) != NULL) {
		char *tempStr = groupToJSON(g);
		buffer = realloc(buffer, strlen(buffer) + strlen(tempStr) + 10);
		strcat(buffer, tempStr);
		free(tempStr); //String allocated by groupToJSON is dynamic
		strcat(buffer, ",");
	}
	buffer[strlen(buffer) - 1] = '\0';
	strcat(buffer, "]");
	
	return buffer;
}

/* Bonus Functions but needed for A3 */

SVGimage* JSONtoSVG(const char* svgString) {
	if (svgString == NULL) {
		return NULL; //Do nothing
	}
	
	//Allocating the image
	SVGimage *image = malloc(sizeof(SVGimage));
	image->rectangles = initializeList(&rectangleToString, &deleteRectangle, &compareRectangles);
	image->circles = initializeList(&circleToString, &deleteCircle, &compareCircles);
	image->paths = initializeList(&pathToString, &deletePath, &comparePaths);
	image->groups = initializeList(&groupToString, &deleteGroup, &compareGroups);
	image->otherAttributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);
	
	strcpy(image->namespace, "http://www.w3.org/2000/svg");
	strcpy(image->title, "");
	strcpy(image->description, "");
	
	char str[600];
	strcpy(str, (char*)svgString);
	char* token = strtok(str, ":\"");
	int titleToggle = 0; //if 1, next line is a value
	int descToggle = 0; //if 1, next line is a value
	
    while (token != NULL) {
        //printf("%s\n", token);
        if (titleToggle == 1) { //Title found!
			if ( !(strcmp(token, ",") == 0 || strcmp(token, "}") == 0)) {
				strncpy(image->title, token, 255);
			}
			titleToggle = 0;
		} else if (descToggle == 1) { //Description found!
			if ( !(strcmp(token, ",") == 0 || strcmp(token, "}") == 0)) {
				strncpy(image->description, token, 255);
			}
			descToggle = 0;
		}
		
		//Checking for title or desc
        if (strcmp(token, "title") == 0) {
			titleToggle = 1;
		}
		if (strcmp(token, "descr") == 0) {
			descToggle = 1;
		}

		//Next token
        token = strtok(NULL, ":\"");
    }
    
    return image;
}
Rectangle* JSONtoRect(const char* svgString) {
	if (svgString == NULL) {
		return NULL;
	}
	
	//Initalizing rect
	Rectangle *rect = malloc(sizeof(Rectangle));
	rect->x = -1;
	rect->y = -1;
	rect->width = -1;
	rect->height = -1;
	strcpy(rect->units, "");
	rect->otherAttributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);
	
	//Parsing the string
	char str[600];
	strcpy(str, (char*)svgString);
	char* token = strtok(str, "\":");

	int xToggle = 0; //if 1, next line is a value
	int yToggle = 0; //if 1, next line is a value
	int wToggle = 0; //if 1, next line is a value
	int hToggle = 0; //if 1, next line is a value
	int uToggle = 0; //if 1, next line is a value
	
    while (token != NULL) {
        //printf("%s\n", token);
		if (strcmp(token, ",") != 0) {
			if (xToggle == 1 && isdigit(token[0]) != 0) {
				token[strlen(token) - 1] = '\0';
				rect->x = (float)atof(token);
			}
			if (yToggle == 1 && isdigit(token[0]) != 0) {
				token[strlen(token) - 1] = '\0';
				rect->y = (float)atof(token);
			}
			if (wToggle == 1 && isdigit(token[0]) != 0) {
				token[strlen(token) - 1] = '\0';
				rect->width = (float)atof(token);
			}
			if (hToggle == 1 && isdigit(token[0]) != 0) {
				token[strlen(token) - 1] = '\0';
				rect->height = (float)atof(token);
			}
			if (uToggle == 1 && token[0] != '}') {
				strncpy(rect->units, token, 49);
			}
			xToggle = yToggle = wToggle = hToggle = uToggle = 0;
		}
		
		//Checking for correct token
        if (strcmp(token, "x") == 0) {
			xToggle = 1;
		} else if (strcmp(token, "y") == 0) {
			yToggle = 1;
		} else if (strcmp(token, "w") == 0) {
			wToggle = 1;
		} else if (strcmp(token, "h") == 0) {
			hToggle = 1;
		} else if (strcmp(token, "units") == 0) {
			uToggle = 1;
		}

		//Next token
        token = strtok(NULL, ":\"");
    }
	
	//Missing values
	if (rect->x == -1 || rect->y == -1 || rect->height == -1 || rect->width ==  -1) {
		freeList(rect->otherAttributes);
		free(rect);
		return NULL;
	}

	return rect;
}
Circle* JSONtoCircle(const char* svgString) {
	if (svgString == NULL) {
		return NULL;
	}
	
	//Initalizing circle
	Circle *circle = malloc(sizeof(Circle));
	circle->cx = -1;
	circle->cy = -1;
	circle->r = -1;
	strcpy(circle->units, "");
	circle->otherAttributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);
	
	//Parsing the string
	char str[600];
	strcpy(str, (char*)svgString);
	char* token = strtok(str, "\":");

	int cxToggle = 0; //if 1, next line is a value
	int cyToggle = 0; //if 1, next line is a value
	int rToggle = 0; //if 1, next line is a value
	int uToggle = 0; //if 1, next line is a value
	
    while (token != NULL) {
        //printf("%s\n", token);
		if (strcmp(token, ",") != 0) {
			if (cxToggle == 1 && isdigit(token[0]) != 0) {
				token[strlen(token) - 1] = '\0';
				circle->cx = (float)atof(token);
			}
			if (cyToggle == 1 && isdigit(token[0]) != 0) {
				token[strlen(token) - 1] = '\0';
				circle->cy = (float)atof(token);
			}
			if (rToggle == 1 && isdigit(token[0]) != 0) {
				token[strlen(token) - 1] = '\0';
				circle->r = (float)atof(token);
			}
			if (uToggle == 1 && token[0] != '}') {
				strncpy(circle->units, token, 49);
			}
			cxToggle = cyToggle = rToggle = uToggle = 0;
		}
		
		//Checking for correct token
        if (strcmp(token, "cx") == 0) {
			cxToggle = 1;
		} else if (strcmp(token, "cy") == 0) {
			cyToggle = 1;
		} else if (strcmp(token, "r") == 0) {
			rToggle = 1;
		} else if (strcmp(token, "units") == 0) {
			uToggle = 1;
		}

		//Next token
        token = strtok(NULL, ":\"");
    }
	
	//Missing values
	if (circle->cx == -1 || circle->cy == -1 || circle->r == -1) {
		freeList(circle->otherAttributes);
		free(circle);
		return NULL;
	}

	return circle;
}

/* Functions wrappers for serverside code */

//Validate File
char* validateSVG(char* fileName) {
	SVGimage *image = createValidSVGimage(fileName, "parser/svg.xsd");
	
	char *buffer = malloc(50);
	strcpy(buffer, "error");
	
	if (image != NULL) { //Image is valid!
		strcpy(buffer, "success");
		deleteSVGimage(image);
	} else { //Remove file if its invalid
		remove(fileName);
	}
	
	return buffer;
}

//Returns a JSON string of an a specified SVG image if its valid.
char* getSVGProperties(char* fileName) {
	SVGimage *image = createValidSVGimage(fileName, "parser/svg.xsd");
	
	char *buffer = malloc(50);
	strcpy(buffer, "error");
	
	if (image != NULL) { //Image is valid!
		free(buffer);
		buffer = SVGtoJSON(image);
	}
	
	deleteSVGimage(image);
	
	return buffer;
}
char* getSVGTitleDesc(char* fileName) {
	SVGimage *image = createValidSVGimage(fileName, "parser/svg.xsd");
	
	char *buffer = malloc(1000);
	strcpy(buffer, "error");
	
	if (image != NULL) { //Image is valid!
		free(buffer);
		buffer = malloc(1000);
		
		sprintf(buffer, "{\"title\":\"%s\",\"descr\":\"%s\"}", image->title, image->description);
	}
	
	deleteSVGimage(image);
	
	return buffer;
}
char* getSVGRects(char* fileName) {
	SVGimage *image = createValidSVGimage(fileName, "parser/svg.xsd");
	
	char *buffer = malloc(50);
	strcpy(buffer, "error");
	
	if (image != NULL) { //Image is valid!
		free(buffer);
		
		buffer = rectListToJSON(image->rectangles);
	}
	
	deleteSVGimage(image);
	
	return buffer;
}
char* getSVGCircs(char* fileName) {
	SVGimage *image = createValidSVGimage(fileName, "parser/svg.xsd");
	
	char *buffer = malloc(50);
	strcpy(buffer, "error");
	
	if (image != NULL) { //Image is valid!
		free(buffer);
		
		buffer = circListToJSON(image->circles);
	}
	
	deleteSVGimage(image);
	
	return buffer;
}
char* getSVGPaths(char* fileName) {
	SVGimage *image = createValidSVGimage(fileName, "parser/svg.xsd");
	
	char *buffer = malloc(50);
	strcpy(buffer, "error");
	
	if (image != NULL) { //Image is valid!
		free(buffer);
		
		buffer = pathListToJSON(image->paths);
	}
	
	deleteSVGimage(image);
	
	return buffer;
}
char* getSVGGroups(char* fileName) {
	SVGimage *image = createValidSVGimage(fileName, "parser/svg.xsd");
	
	char *buffer = malloc(50);
	strcpy(buffer, "error");
	
	if (image != NULL) { //Image is valid!
		free(buffer);
		
		buffer = groupListToJSON(image->groups);
	}
	
	deleteSVGimage(image);
	
	return buffer;
}

//Changing title/description
char* changeTitle(char* fileName, char* newTitle) {
	SVGimage *image = createValidSVGimage(fileName, "parser/svg.xsd");
	
	char *buffer = malloc(50);
	sprintf(buffer, "error");
	
	if (image != NULL) { //Image is valid!
		strcpy(image->title, newTitle);
		if (validateSVGimage(image, "parser/svg.xsd") == true) { //SVG is valid
			if (writeSVGimage(image, fileName) == true) { //SVG wrote correctly
				sprintf(buffer, "success");
			}
		}
	}
	
	deleteSVGimage(image);
	
	return buffer;
}
char* changeDescription(char* fileName, char* newDescription) {
	SVGimage *image = createValidSVGimage(fileName, "parser/svg.xsd");
	
	char *buffer = malloc(50);
	sprintf(buffer, "error");
	
	if (image != NULL) { //Image is valid!
		strcpy(image->description, newDescription);
		if (validateSVGimage(image, "parser/svg.xsd") == true) { //SVG is valid
			if (writeSVGimage(image, fileName) == true) { //SVG wrote correctly
				sprintf(buffer, "success");
			}
		}
	}
	
	deleteSVGimage(image);
	
	return buffer;
}

//Getting attributes
char* getRectAttributes(char* fileName, int num) {
	SVGimage *image = createValidSVGimage(fileName, "parser/svg.xsd");
	
	char *buffer = malloc(50);
	strcpy(buffer, "error");
	
	if (image != NULL) { //Image is valid!
		free(buffer);
		
		//Finding rectangle
		Rectangle *elem;
		ListIterator iter = createIterator(image->rectangles);
		int i = 1;
		while ((elem = nextElement(&iter)) != NULL) {
			if (i == num) {
				break;
			}
			i++;
		}
		if (i != num) { //Rectangle couldnt be found
			deleteSVGimage(image);
			return buffer;
		}
		
		buffer = attrListToJSON(elem->otherAttributes);
	}
	
	deleteSVGimage(image);
	
	return buffer;
}
char* getCircAttributes(char* fileName, int num) {
	SVGimage *image = createValidSVGimage(fileName, "parser/svg.xsd");
	
	char *buffer = malloc(50);
	strcpy(buffer, "error");
	
	if (image != NULL) { //Image is valid!
		free(buffer);
		
		//Finding circle
		Circle *elem;
		ListIterator iter = createIterator(image->circles);
		int i = 1;
		while ((elem = nextElement(&iter)) != NULL) {
			if (i == num) {
				break;
			}
			i++;
		}
		if (i != num) { //Circle couldnt be found
			deleteSVGimage(image);
			return buffer;
		}
		
		buffer = attrListToJSON(elem->otherAttributes);
	}
	
	deleteSVGimage(image);
	
	return buffer;
}
char* getPathAttributes(char* fileName, int num) {
	SVGimage *image = createValidSVGimage(fileName, "parser/svg.xsd");
	
	char *buffer = malloc(50);
	strcpy(buffer, "error");
	
	if (image != NULL) { //Image is valid!
		free(buffer);
		
		//Finding path
		Path *elem;
		ListIterator iter = createIterator(image->paths);
		int i = 1;
		while ((elem = nextElement(&iter)) != NULL) {
			if (i == num) {
				break;
			}
			i++;
		}
		if (i != num) { //Path couldnt be found
			deleteSVGimage(image);
			return buffer;
		}
		
		buffer = attrListToJSON(elem->otherAttributes);
	}
	
	deleteSVGimage(image);
	
	return buffer;
}
char* getGroupAttributes(char* fileName, int num) {
	SVGimage *image = createValidSVGimage(fileName, "parser/svg.xsd");
	
	char *buffer = malloc(50);
	strcpy(buffer, "error");
	
	if (image != NULL) { //Image is valid!
		free(buffer);
		
		//Finding group
		Group *elem;
		ListIterator iter = createIterator(image->groups);
		int i = 1;
		while ((elem = nextElement(&iter)) != NULL) {
			if (i == num) {
				break;
			}
			i++;
		}
		if (i != num) { //Group couldnt be found
			deleteSVGimage(image);
			return buffer;
		}
		
		buffer = attrListToJSON(elem->otherAttributes);
	}
	
	deleteSVGimage(image);
	
	return buffer;
}

//Editing Attributes
char* editAttributes(char* fileName, int num, char* type, char* attrName, char* attrValue) {
	SVGimage *image = createValidSVGimage(fileName, "parser/svg.xsd");
	
	char *buffer = malloc(50);
	strcpy(buffer, "error");
	
	if (image != NULL) { //Image is valid!
		//Create attribute to add
		Attribute *a = malloc(sizeof(Attribute));
		a->name = malloc(strlen(attrName) + 100);
		a->value = malloc(strlen(attrValue) + 100);
		strcpy(a->name, attrName);
		strcpy(a->value, attrValue);
		
		//Add attribute created depending on type
		if (strcmp(type, "SVG_IMAGE") == 0) {
			setAttribute(image, SVG_IMAGE, num-1, a);
		} else if (strcmp(type, "RECT") == 0) {
			setAttribute(image, RECT, num-1, a);
		} else if (strcmp(type, "CIRC") == 0) {
			setAttribute(image, CIRC, num-1, a);
		} else if (strcmp(type, "PATH") == 0) {
			setAttribute(image, PATH, num-1, a);
		} else if (strcmp(type, "GROUP") == 0) {
			setAttribute(image, GROUP, num-1, a);
		}

		//Validate and write SVG
		if (validateSVGimage(image, "parser/svg.xsd") == true) { //SVG is valid
			if (writeSVGimage(image, fileName) == true) { //SVG wrote correctly
				sprintf(buffer, "success");
			}
		}
	}
	
	deleteSVGimage(image);
	
	return buffer;
}

//Create SVG
char* createSVG(char* fileName) {
	char *buffer = malloc(50);
	sprintf(buffer, "error");
	
	//Allocating the image
	SVGimage *image = malloc(sizeof(SVGimage));
	image->rectangles = initializeList(&rectangleToString, &deleteRectangle, &compareRectangles);
	image->circles = initializeList(&circleToString, &deleteCircle, &compareCircles);
	image->paths = initializeList(&pathToString, &deletePath, &comparePaths);
	image->groups = initializeList(&groupToString, &deleteGroup, &compareGroups);
	image->otherAttributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);
	
	strcpy(image->namespace, "http://www.w3.org/2000/svg");
	strcpy(image->title, "");
	strcpy(image->description, "");

	//Validate and write SVG
	if (validateSVGimage(image, "parser/svg.xsd") == true) { //SVG is valid
		if (writeSVGimage(image, fileName) == true) { //SVG wrote correctly
			sprintf(buffer, "success");
		}
	}
	
	deleteSVGimage(image);
	
	return buffer;
}

//Add shape to SVG
char* addRectangle(char* fileName, char* JSON) {
	SVGimage *image = createValidSVGimage(fileName, "parser/svg.xsd");
	
	char *buffer = malloc(50);
	strcpy(buffer, "error");
	
	if (image != NULL) { //Image is valid!
		Rectangle *r = JSONtoRect(JSON); //Converting JSON to rectangle
		if (r != NULL) {
			addComponent(image, RECT, r); //Adding shape
		} else {
			deleteSVGimage(image);
			return buffer;
		}
		
		//Write SVG only if valid
		if (validateSVGimage(image, "parser/svg.xsd") == true) { //SVG is valid
			if (writeSVGimage(image, fileName) == true) { //SVG wrote correctly
				strcpy(buffer, "success");
			}
		}
	}
	
	deleteSVGimage(image);
	return buffer;
}
char* addCircle(char* fileName, char* JSON) {
	SVGimage *image = createValidSVGimage(fileName, "parser/svg.xsd");
	
	char *buffer = malloc(50);
	strcpy(buffer, "error");
	
	if (image != NULL) { //Image is valid!
		Circle *c = JSONtoCircle(JSON); //Converting JSON to rectangle
		if (c != NULL) {
			addComponent(image, CIRC, c); //Adding shape
		} else {
			deleteSVGimage(image);
			return buffer;
		}
		
		//Write SVG only if valid
		if (validateSVGimage(image, "parser/svg.xsd") == true) { //SVG is valid
			if (writeSVGimage(image, fileName) == true) { //SVG wrote correctly
				strcpy(buffer, "success");
			}
		}
	}
	
	deleteSVGimage(image);

	return buffer;
}

//Scale Shape
char* scaleCircles(char* fileName, float scaleFactor) {
	SVGimage *image = createValidSVGimage(fileName, "parser/svg.xsd");
	
	char *buffer = malloc(50);
	strcpy(buffer, "error");
	
	if (image != NULL) { //Image is valid!
		Circle *elem;
		List *list = getCircles(image);
		ListIterator iter = createIterator(list);
		while ((elem = nextElement(&iter)) != NULL) {
			elem->r *= scaleFactor;
		}
		freeList(list);
		
		//Write SVG only if valid
		if (validateSVGimage(image, "parser/svg.xsd") == true) { //SVG is valid
			if (writeSVGimage(image, fileName) == true) { //SVG wrote correctly
				strcpy(buffer, "success");
			}
		}
	}
	
	deleteSVGimage(image);

	return buffer;
}
char* scaleRectangles(char* fileName, float scaleFactor) {
	SVGimage *image = createValidSVGimage(fileName, "parser/svg.xsd");
	
	char *buffer = malloc(50);
	strcpy(buffer, "error");
	
	if (image != NULL) { //Image is valid!
		Rectangle *elem;
		List *list = getRects(image);
		ListIterator iter = createIterator(list);
		while ((elem = nextElement(&iter)) != NULL) {
			elem->width *= scaleFactor;
			elem->height *= scaleFactor;
		}
		freeList(list);
		
		//Write SVG only if valid
		if (validateSVGimage(image, "parser/svg.xsd") == true) { //SVG is valid
			if (writeSVGimage(image, fileName) == true) { //SVG wrote correctly
				strcpy(buffer, "success");
			}
		}
	}
	
	deleteSVGimage(image);

	return buffer;
}
