#include "xwriter.h"

int main(int argc, char** argv)
{
    xmlDocPtr doc = NULL;
    xmlNodePtr rootNode = NULL;
    xmlNodePtr childNode = NULL;
    xmlNodePtr childNode2 = NULL;
    xmlNodePtr sub_child = NULL;
    xmlNodePtr comment_node = NULL;
    xmlChar* out = NULL;
    int len = 0;
    
    doc = xmlCreateWriter("root", NULL);
    if(doc) 
    {
        rootNode = xmlGetRootNode(doc);
        childNode = xmlAddChildNode(rootNode,XML_ELEMENT_NODE, "child1", "this is 中文child1");
//        xmlSetNodeProperty(childNode, "name", "hello中文");

        comment_node = xmlAddChildNode(rootNode,XML_COMMENT_NODE, "comment", "this is comment for child1");
        xmlSetNodeProperty(comment_node, "name", "hello中文");

        childNode2 = xmlAddChildNode(rootNode, XML_ELEMENT_NODE,"child2", "this is child2");
        xmlSetNodeProperty(childNode2, "name", "hello child2");
		xmlSetNodeProperty(childNode2, "name2", "hello child22");

        sub_child = xmlAddChildNode(childNode2,XML_ELEMENT_NODE, "sub_child", "");
        xmlSetNodeProperty(sub_child, "name", "hello sub_child");

        xmlDumpToFile(doc, "testwriter.xml", "utf-8");
        
        xmlDumpToMemory(doc, &out, &len, "utf-8");
        if(out)
        {
            printf("%s", out);
            xmlFree(out);
        }
        xmlReleaseDoc(doc);
    }
    return 0;
}


