#include <stdio.h>
#include "XmlHelper.hpp"
#include <boost/tokenizer.hpp>

XmlHelper::XmlHelper(const xmlNode *__node)
    : node(__node)
{
}

NodeVector XmlHelper::getChildren() const
{
  NodeVector children;

  for (auto cur_node = node->children; cur_node; cur_node = cur_node->next)
  {
    if (cur_node->type == XML_ELEMENT_NODE)
      children.push_back(cur_node);
  }

  return children;
}

NodeVector XmlHelper::getChildren(String nome) const
{
  NodeVector children;

  for (auto cur_node = node->children; cur_node; cur_node = cur_node->next)
  {
    if (cur_node->type == XML_ELEMENT_NODE)
    {
      String nomeNodo = (char *)cur_node->name;
      if (nome == nomeNodo)
        children.push_back(cur_node);
    }
  }

  return children;
}

StringMap XmlHelper::getAttributes() const
{
  StringMap attributes;

  for (auto attr = node->properties; attr; attr = attr->next)
  {
    if (attr->type != XML_ATTRIBUTE_NODE || attr->children == nullptr || attr->children->content == nullptr)
      continue;

    String nomeAttr = (char *)attr->name;
    String valoreAttr = (char *)attr->children->content;
    attributes[nomeAttr] = valoreAttr;
  }

  return attributes;
}

String XmlHelper::getAttribute(String nome) const
{
  for (auto attr = node->properties; attr; attr = attr->next)
  {
    if (attr->type != XML_ATTRIBUTE_NODE || attr->children == nullptr || attr->children->content == nullptr)
      continue;

    String nomeAttr = (char *)attr->name;
    String valoreAttr = (char *)attr->children->content;

    if (nome == nomeAttr)
      return valoreAttr;
  }

  return "";
}

const NodePtr XmlHelper::findElementXml(String nome, const xmlNode *from /*= NULL*/) const
{
  if (from == NULL)
    from = node->children;

  if (from == NULL)
    return NULL;

  for (auto cur_node = from; cur_node; cur_node = cur_node->next)
  {
    if (cur_node->type == XML_ELEMENT_NODE)
    {
      String nomeNodo = (char *)cur_node->name;
      if (nome == nomeNodo)
        return (NodePtr)cur_node;
    }

    if (cur_node->children != NULL)
    {
      const NodePtr rv = findElementXml(nome, cur_node->children);
      if (rv != NULL)
        return rv;
    }
  }

  return NULL;
}

String XmlHelper::getContent() const
{
  for (xmlNode *n = node->children; n; n = n->next)
  {
    if (n->type == XML_TEXT_NODE && n->content != NULL)
    {
      return (char *)n->content;
    }
  }

  return "";
}

bool XmlHelper::findElementXmlContent(String nome, String &content) const
{
  const NodePtr node = findElementXml(nome);
  if (node == NULL || node->children == NULL)
    return false;

  for (xmlNode *n = node->children; n; n = n->next)
  {
    if (n->type == XML_TEXT_NODE && n->content != NULL)
    {
      content = (char *)n->content;
      return true;
    }
  }

  return false;
}

bool XmlHelper::findElementXmlContent(String nome, int &content) const
{
  String tmp;
  if (findElementXmlContent(nome, tmp))
  {
    content = atoi(tmp.c_str());
    return true;
  }

  return false;
}

const NodePtr XmlHelper::findPathXml(String path, const xmlNode *from /*= NULL*/) const
{
  if (from == NULL)
    from = node->children;

  if (from == NULL)
    return NULL;

  auto pos = path.find('/');
  String nome = pos == String::npos ? path : path.substr(0, pos);
  String npat = pos == String::npos ? "" : path.substr(pos + 1);

  for (auto cur_node = from; cur_node; cur_node = cur_node->next)
  {
    if (cur_node->type == XML_ELEMENT_NODE)
    {
      String nomeNodo = (char *)cur_node->name;
      if (nome == nomeNodo)
      {
        if (pos == String::npos)
          return (NodePtr)cur_node;

        if (cur_node->children != NULL)
        {
          const NodePtr rv = findPathXml(npat, cur_node->children);
          if (rv != NULL)
            return rv;
        }
      }
    }
  }

  return NULL;
}

bool XmlHelper::findPathXmlContent(String path, String &content) const
{
  const NodePtr node = findPathXml(path);
  if (node == NULL || node->children == NULL)
    return false;

  for (xmlNode *n = node->children; n; n = n->next)
  {
    if (n->type == XML_TEXT_NODE && n->content != NULL)
    {
      content = (char *)n->content;
      return true;
    }
  }

  return false;
}

bool XmlHelper::findPathXmlContent(String path, int &content) const
{
  String tmp;
  if (findPathXmlContent(path, tmp))
  {
    content = atoi(tmp.c_str());
    return true;
  }

  return false;
}
