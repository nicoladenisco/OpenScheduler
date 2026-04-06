#ifndef __XMLHELPER_H
#define __XMLHELPER_H

#include "common.hpp"

using NodePtr = xmlNode*;
using NodeVector = std::vector<NodePtr>;

class XmlHelper
{
public:
  XmlHelper(const xmlNode *node);

  NodeVector getChildren() const;
  NodeVector getChildren(String nome) const;
  const NodePtr findElementXml(String nome, const xmlNode *from = NULL) const;
  bool findElementXmlContent(String nome, String &content) const;
  bool findElementXmlContent(String nome, int &content) const;
  const NodePtr findPathXml(String path, const xmlNode *from = NULL) const;
  bool findPathXmlContent(String path, String &content) const;
  bool findPathXmlContent(String path, int &content) const;

  StringMap getAttributes() const;
  String getAttribute(String nome) const;
  String getContent() const;

private:
  const xmlNode *node;
};

using XmlHelperVector = std::vector<XmlHelper>;

#endif
