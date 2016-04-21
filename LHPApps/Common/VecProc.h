/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: VecProc.h,v $
  Language:  C++
  Date:      $Date: 2009-05-19 14:29:52 $
  Version:   $Revision: 1.1 $
  Authors:   Fedor Moiseev
==========================================================================
  Copyright (c) 2001/2007 
  ULB - Universite Libre de Bruxelles (www.ulb.ac.be)
=========================================================================*/

#ifndef __VECPROC__H__
#define __VECPROC__H__

#include <strstream>
#include <vector>
#include <map>
#include <string>
#include <math.h>
#include "vectors.h"



template <class Type>
class Param
{
public:
  enum VarType
  {
    VECTOR = 0,
    SCALAR
  };
  Param(VarType type, bool dn = false, bool up = false):m_type(type), m_dn(dn), m_up(up), m_valid(true){}
  //Param(const char *name, VarType type, bool dn = false, bool up = false):m_type(type), m_dn(dn), m_up(up),m_name(name){}
  const std::string& GetName(){return m_name;}
  VarType            GetType(){return m_type;}
  Type&              GetScalar(){return m_value[0];}
  const Type&        GetScalar()const{return m_value[0];}
  bool               IsUpLimited()const{return m_up;}
  bool               IsDnLimited()const{return m_dn;}
  bool&              UpLimited(){return m_up;}
  bool&              DnLimited(){return m_dn;}
  Type&              GetUpLimit(){return m_upLimit;}
  const Type&        GetUpLimit()const{return m_upLimit;}
  Type&              GetDnLimit(){return m_dnLimit;}
  const Type&        GetDnLimit()const{return m_dnLimit;}
  V3d<Type>&         GetVector(){return m_value;}
  const V3d<Type>&   GetVector()const{return m_value;}
  bool&              GetValid(){return m_valid;}
  bool               IsValid()const{return m_valid;}
private:
  Type              m_upLimit;
  Type              m_dnLimit;
  bool              m_up;
  bool              m_dn;
  V3d<Type>         m_value;
  bool              m_valid;
  const VarType     m_type;
  //const std::string m_name;
};

template <class Type>
struct oneParam
{
  oneParam(bool v, bool i, bool c, bool o, Param<Type> **p):vector(v),input(i), constant(c), optional(o), param(p){}
  bool        vector;
  bool        input;
  bool        constant;
  bool        optional;
  Param<Type> **param;
};

/*#define FIELDS_BEGIN(Type)  static oneParam<typename Type> m_ops[] = {
#define FIELDS_END()  };\
virtual std::vector<oneParam<Type> > getFields()  \
{                                                   \
  std::vector<oneParam<Type> > fields;
  for(unsigned i = 0; i < sizeof(m_ops) / sizeof(m_ops[0]); i++) fields.push_back(m_ops[i]);\
  return fields;                             \
}*/


/*#define FIELDS_BEGIN(Type)                          \
virtual std::vector<oneParam<Type> > getFields()  \
{                                                   \
std::vector<oneParam<Type> > fields;              oneParam<typename Type> op[] = {
#define FIELDS_END()                               \
};for(unsigned i = 0; i < sizeof(op) / sizeof(op[0]); i++) fields.push_back(op[i]);\
return fields;                             \
}*/

/*#define FIELDS_BEGIN(Type)                          \
virtual std::vector<oneParam<Type> > getFields()  \
{                                                   \
std::vector<oneParam<Type> > fields;              oneParam<typename Type> op[] = {
#define FIELDS_END()                               \
};for(unsigned i = 0; i < sizeof(op) / sizeof(op[0]); i++) fields.push_back(op[i]);\
return fields;                             \
}*/



#define DEFINE_FIELD(name,vector,input) do{m_ops.push_back(oneParam<Type>(vector, input, false, false, &m_##name));m_##name = NULL;}while(0)
#define DEFINE_VFIELDI(name)   do{m_ops.push_back(oneParam<Type>(true,  true,  false, false, &m_##name));m_##name = NULL;}while(0)
#define DEFINE_VFIELDO(name)   do{m_ops.push_back(oneParam<Type>(true,  false, false, false, &m_##name));m_##name = NULL;}while(0)
#define DEFINE_SFIELDI(name)   do{m_ops.push_back(oneParam<Type>(false, true,  false, false, &m_##name));m_##name = NULL;}while(0)
#define DEFINE_SFIELDI(name)   do{m_ops.push_back(oneParam<Type>(false, true,  false, false, &m_##name));m_##name = NULL;}while(0)
#define DEFINE_SFIELDO(name)   do{m_ops.push_back(oneParam<Type>(false, false, false, false, &m_##name));m_##name = NULL;}while(0)
#define DEFINE_SFIELDIC(name)  do{m_ops.push_back(oneParam<Type>(false, true,  true,  false, &m_##name));m_##name = NULL;}while(0)
#define DEFINE_SFIELDICO(name) do{m_ops.push_back(oneParam<Type>(false, true,  true,  true,  &m_##name));m_##name = NULL;}while(0)

template <class Type>
class Oper
{
public:
  Oper(){}
  virtual bool checkInputFields(){for(unsigned i = 0; i < m_ops.size(); i++){if(m_ops[i].input && !m_ops[i].param[0]->IsValid())return false;}return true;}
  virtual void validateOutputFields(){for(unsigned i = 0; i < m_ops.size(); i++){if(!m_ops[i].input)m_ops[i].param[0]->GetValid()=true;}}
  virtual bool postRead() = 0;
  virtual bool process() = 0;
  virtual std::vector<oneParam<Type> >& getFields(){return m_ops;}
protected:
  std::vector<oneParam<Type> > m_ops;
};

template <class Type>
class AssignVector : public Oper<Type>
{
public:
  AssignVector()
  {
    DEFINE_FIELD(out, true, false);
    DEFINE_FIELD(in1, true, true);
  }
  bool postRead(){return true;}
  bool process(){if(!checkInputFields())return false;m_out->GetVector() = m_in1->GetVector();validateOutputFields();return true;}
private:
  Param<Type>       *m_out;
  Param<Type>       *m_in1;
};

template <class Type>
class AssignScalar : public Oper<Type>
{
public:
  AssignScalar()
  {
    DEFINE_FIELD(out, false, false);
    DEFINE_FIELD(in1, false, true);
  }
  bool postRead(){return true;}
  bool process(){if(!checkInputFields())return false;m_out->GetScalar() = m_in1->GetScalar();validateOutputFields();return true;}
private:
  Param<Type>       *m_out;
  Param<Type>       *m_in1;
};


template <class Type>
class CrossProduct : public Oper<Type>
{
public:
  CrossProduct()
  {
    DEFINE_FIELD(out, true, false);
    DEFINE_FIELD(in1, true, true);
    DEFINE_FIELD(in2, true, true);
  }
  bool postRead(){return true;}
  bool process(){if(!checkInputFields())return false;m_out->GetVector() = m_in1->GetVector() ^ m_in2->GetVector();validateOutputFields();return true;}
private:
  Param<Type>       *m_out;
  Param<Type>       *m_in1;
  Param<Type>       *m_in2;
};

template <class Type>
class DotProduct : public Oper<Type>
{
public:
  DotProduct()
  {
    DEFINE_FIELD(out, false, false);
    DEFINE_FIELD(in1, true, true);
    DEFINE_FIELD(in2, true, true);
  }
  bool postRead(){return true;}
  bool process(){if(!checkInputFields())return false;m_out->GetScalar() = m_in1->GetVector() * m_in2->GetVector();validateOutputFields();return true;}
private:
  Param<Type>       *m_out;
  Param<Type>       *m_in1;
  Param<Type>       *m_in2;
};
template <class Type>
class AddScalar : public Oper<Type>
{
public:
  AddScalar()
  {
    DEFINE_FIELD(out, false, false);
    DEFINE_FIELD(in1, false, true);
    DEFINE_FIELD(in2, false, true);
  }
  bool postRead(){return true;}
  bool process(){if(!checkInputFields())return false;m_out->GetScalar() = m_in1->GetScalar() + m_in2->GetScalar();validateOutputFields();return true;}
private:
  Param<Type>       *m_out;
  Param<Type>       *m_in1;
  Param<Type>       *m_in2;
};
template <class Type>
class SubScalar : public Oper<Type>
{
public:
  SubScalar()
  {
    DEFINE_FIELD(out, false, false);
    DEFINE_FIELD(in1, false, true);
    DEFINE_FIELD(in2, false, true);
  }
  bool postRead(){return true;}
  bool process(){if(!checkInputFields())return false;m_out->GetScalar() = m_in1->GetScalar() - m_in2->GetScalar();validateOutputFields();return true;}
private:
  Param<Type>       *m_out;
  Param<Type>       *m_in1;
  Param<Type>       *m_in2;
};
template <class Type>
class MulScalar : public Oper<Type>
{
public:
  MulScalar()
  {
    DEFINE_FIELD(out, false, false);
    DEFINE_FIELD(in1, false, true);
    DEFINE_FIELD(in2, false, true);
  }
  bool postRead(){return true;}
  bool process(){if(!checkInputFields())return false;m_out->GetScalar() = m_in1->GetScalar() * m_in2->GetScalar();validateOutputFields();return true;}
private:
  Param<Type>       *m_out;
  Param<Type>       *m_in1;
  Param<Type>       *m_in2;
};
template <class Type>
class DivScalar : public Oper<Type>
{
public:
  DivScalar()
  {
    DEFINE_FIELD(out, false, false);
    DEFINE_FIELD(in1, false, true);
    DEFINE_FIELD(in2, false, true);
  }
  bool postRead(){return true;}
  bool process(){if(!checkInputFields())return false;m_out->GetScalar() = m_in1->GetScalar() / m_in2->GetScalar();validateOutputFields();return true;}
private:
  Param<Type>       *m_out;
  Param<Type>       *m_in1;
  Param<Type>       *m_in2;
};
template <class Type>
class AddVector : public Oper<Type>
{
public:
  AddVector()
  {
    DEFINE_FIELD(out, true, false);
    DEFINE_FIELD(in1, true, true);
    DEFINE_FIELD(in2, true, true);
  }
  bool postRead(){return true;}
  bool process(){if(!checkInputFields())return false;m_out->GetVector() = m_in1->GetVector() + m_in2->GetVector();validateOutputFields();return true;}
private:
  Param<Type>       *m_out;
  Param<Type>       *m_in1;
  Param<Type>       *m_in2;
};
template <class Type>
class SubVector : public Oper<Type>
{
public:
  SubVector()
  {
    DEFINE_FIELD(out, true, false);
    DEFINE_FIELD(in1, true, true);
    DEFINE_FIELD(in2, true, true);
  }
  bool postRead(){return true;}
  bool process(){if(!checkInputFields())return false;m_out->GetVector() = m_in1->GetVector() - m_in2->GetVector();validateOutputFields();return true;}
private:
  Param<Type>       *m_out;
  Param<Type>       *m_in1;
  Param<Type>       *m_in2;
};

template <class Type>
class LineComb : public Oper<Type>
{
public:
  LineComb()
  {
    DEFINE_FIELD(out, true, false);
    DEFINE_FIELD(in1s, false, true);
    DEFINE_FIELD(in1v, true, true);
    DEFINE_FIELD(in2s, false, true);
    DEFINE_FIELD(in2v, true, true);
  }
  bool postRead(){return true;}
  bool process()
  {
    if(!checkInputFields())
      return false;
    V3d<Type> p1 = m_in1s->GetScalar() * m_in1v->GetVector();
    V3d<Type> p2 = m_in2s->GetScalar() * m_in2v->GetVector();
    m_out->GetVector() = p1 + p2;
    validateOutputFields();
    return true;
  }
private:
  Param<Type>       *m_out;
  Param<Type>       *m_in1v;
  Param<Type>       *m_in1s;
  Param<Type>       *m_in2v;
  Param<Type>       *m_in2s;
};

template <class Type>
class DefVecIn : public Oper<Type>
{
public:
  DefVecIn()
  {
    DEFINE_FIELD(in, true, true);
  }
  bool postRead(){return true;}
  bool process(){if(!checkInputFields())return false;validateOutputFields();return true;}
private:
  Param<Type>       *m_in;
};

template <class Type>
class DefVecOut : public Oper<Type>
{
public:
  DefVecOut()
  {
    DEFINE_FIELD(out, true, false);
  }
  bool postRead(){return true;}
  bool process(){if(!checkInputFields())return false;validateOutputFields();return true;}
private:
  Param<Type>       *m_out;
};

template <class Type>
class DefSclIn : public Oper<Type>
{
public:
  DefSclIn()
  {
    DEFINE_SFIELDICO(dn);
    DEFINE_SFIELDI(in);
    DEFINE_SFIELDICO(up);
  }
  bool postRead();
  bool process(){if(!checkInputFields())return false;validateOutputFields();return true;}
private:
  Param<Type>       *m_dn;
  Param<Type>       *m_in;
  Param<Type>       *m_up;
};
template <class Type>
bool DefSclIn<Type>::postRead()
{
  if(m_dn != NULL) 
  {
    m_in->DnLimited() = true;
    m_in->GetDnLimit() = m_dn->GetScalar();
  }
  if(m_up != NULL) 
  {
    m_in->UpLimited() = true;
    m_in->GetUpLimit() = m_up->GetScalar();
  }
  if(m_in->IsUpLimited() && m_in->IsDnLimited())
  {
    m_in->GetScalar() = (m_in->GetUpLimit() + m_in->GetDnLimit()) / 2;
    if(m_in->GetUpLimit() < m_in->GetDnLimit())
      std::swap(m_in->GetUpLimit(), m_in->GetDnLimit());
  }
  else
  {
    m_in->GetScalar() = 0;
    if(m_in->IsUpLimited() && (m_in->GetScalar() > m_in->GetUpLimit()))
      m_in->GetScalar() = m_in->GetUpLimit();
    if(m_in->IsDnLimited() && (m_in->GetScalar() < m_in->GetDnLimit()))
      m_in->GetScalar() = m_in->GetDnLimit();
  }
  return true;
}


template <class Type>
class DefSclOut : public Oper<Type>
{
public:
  DefSclOut()
  {
    DEFINE_FIELD(out, false, false);
  }
  bool postRead(){return true;}
  bool process(){if(!checkInputFields())return false;validateOutputFields();return true;}
private:
  Param<Type>       *m_out;
};


template <class Type>
class Normalize : public Oper<Type>
{
public:
  Normalize()
  {
    DEFINE_FIELD(out, true, true);
  }
  bool postRead(){return true;}
  bool process(){if(!checkInputFields())return false;Type ln = m_out->GetVector().length2(); if(ln != Type(0)) m_out->GetVector() /= sqrt(ln);validateOutputFields();return true;}
private:
  Param<Type>       *m_out;
};


template <class Type>
class VecManVM
{
public:
  bool        ReadFromFile(const char *filename);
  const std::vector<std::pair<std::string, Param<Type>*> > &getInputs(){return m_inputs;}
  bool        ProcessString(const char *str);
  Param<Type> *GetParam(const char *name);
  bool        GetVector(const char *name, V3d<Type>& output){Param<double>* it = GetParam(name);if(it == NULL || it->GetType() != Param<double>::VECTOR || !it->IsValid())return false;output = it->GetVector();return true;}
  bool        GetScalar(const char *name,     Type&  output){Param<double>* it = GetParam(name);if(it == NULL || it->GetType() != Param<double>::SCALAR || !it->IsValid())return false;output = it->GetScalar();return true;}
  void        Preexecute();
  bool        Execute();
  ~VecManVM(){Clean();}
private:
  void        Clean();
  const char  *SkipSpaces(const char *str);
  bool        ParseCurrentLexem(const char *&str, char *lex);
  bool        IsFloatCorrect(const char *string);
  Param<Type> *ParseVector(const char *name, bool input);
  Param<Type> *ParseScalar(const char *name, bool input, bool constant);
  std::vector<Oper<Type>*>                           m_operators;
  std::map<std::string, Param<Type>*>                m_operands;
  std::vector<Param<Type>*>                          m_constants;
  std::vector<std::pair<std::string, Param<Type>*> > m_inputs;
};

template <class Type>
void VecManVM<Type>::Preexecute()
{
  for(std::map<std::string, Param<Type>*>::iterator it = m_operands.begin(); it != m_operands.end(); ++it)
    it->second->GetValid() = false;
}

template <class Type>
bool VecManVM<Type>::Execute()
{
  bool result = true;
  for(unsigned i = 0; i < m_operators.size(); i++)
  {
    bool opres = m_operators[i]->process();
    result = result && opres;
  }
  return result;
}

template <class Type>
Param<Type> *VecManVM<Type>::GetParam(const char *name)
{  
  std::map<std::string, Param<Type>*>::iterator it = m_operands.find(name);
  if(it == m_operands.end())
    return NULL;
  return it->second;
}


template <class Type>
void VecManVM<Type>::Clean()
{
  for(std::vector<Oper<Type>*>::iterator it = m_operators.begin(); it != m_operators.end(); ++it)
    delete *it;
  m_operators.clear();
  for(std::map<std::string, Param<Type>*>::iterator it = m_operands.begin(); it != m_operands.end(); ++it)
    delete it->second;
  m_operands.clear();
  for(std::vector<Param<Type>*>::iterator it = m_constants.begin(); it != m_constants.end(); ++it)
    delete *it;
  m_constants.clear();
}


template <class Type>
const char *VecManVM<Type>::SkipSpaces(const char *str)
{
  if(str == NULL)
    return NULL;
  while(str[0] && isspace(str[0]))
    str++;
  return str;
}

template <class Type>
bool VecManVM<Type>::ParseCurrentLexem(const char *&str, char *lex)
{
  str = SkipSpaces(str);
  if(str == NULL || lex == NULL)
    return false;
  if(str[0] == '#')
  {
    *lex++ = '#';
  }
  else if(str[0] != '\"')
  {
    //skip first word
    while(str[0] && !isspace(str[0]))
      *lex++ = toupper(*str++);
  }
  else
  {
    str++;
    while(str[0] && str[0] != '\"')
      *lex++ = toupper(*str++);
  }
  lex[0] = '\0';
  return true;
}



template <class Type>
bool VecManVM<Type>::IsFloatCorrect(const char *string)
{
  int nState,nI;

  nState=0;
  for(nI=0;;nI++)
  {
    switch(string[nI])
    {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      if(nState==0||nState==1)
      {
        nState=2;
      }
      else if(nState==3)
      {
        nState=4;
      }
      break;
    case '+':
    case '-':
      if(nState!=0)
      {
        return false;
      }
      nState=1;
      break;
    case '.':
      if(nState==2)
      {
        nState=3;
      }
      else
      {
        return false;
      }
      break;
    case 0:
      return(nState==2||nState==4);
    default:
      return false;
    }
  }
}

template <class Type>
Param<Type> *VecManVM<Type>::ParseVector(const char *name, bool input)
{
  Param<Type> *o = NULL;
  std::map<std::string, Param<Type>*>::iterator  it = m_operands.find(name);
  if(it == m_operands.end())
  {
    o = new Param<Type>(Param<Type>::VECTOR);
    m_operands[name] = o;
    if(input)
      m_inputs.push_back(std::make_pair(name, o));
  }
  else if(it->second->GetType() == Param<Type>::VECTOR)
  {
    o = it->second;
  }
  return o;
}



template <class Type>
Param<Type> *VecManVM<Type>::ParseScalar(const char *name, bool input, bool constant)
{
  Param<Type> *o = NULL;
  if(IsFloatCorrect(name))
  {
    if(input)
    {
      Type num;
      std::istrstream is(name);
      is >> num;
      o = new Param<Type>(Param<Type>::SCALAR);
      o->GetScalar() = num;
      m_constants.push_back(o);
    }
  }
  else if(!constant)
  {
    std::map<std::string, Param<Type>*>::iterator it = m_operands.find(name);
    if(it == m_operands.end())
    {
      o = new Param<Type>(Param<Type>::SCALAR);
      m_operands[name] = o;
      if(input)
        m_inputs.push_back(std::make_pair(name, o));
    }
    else if(it->second->GetType() == Param<Type>::SCALAR)
    {
      o = it->second;
    }
  }
  return o;
}


template <class Type>
bool VecManVM<Type>::ProcessString(const char *pLine)
{
  char      activ[1000];

  if(pLine == NULL)
    return false;

  ParseCurrentLexem(pLine, activ);
  if(activ[0] == '\0' || activ[0] == '#')
    return true;
  Oper<Type> *oper = NULL;
  if(strcmp(activ, "DEFVI") == 0)
  {
    oper = new DefVecIn<Type>;
  }
  else if(strcmp(activ, "DEFVO") == 0)
  {
    oper = new DefVecOut<Type>;
  }
  else if(strcmp(activ, "DEFSI") == 0)
  {
    oper = new DefSclIn<Type>;
  }
  else if(strcmp(activ, "DEFSO") == 0)
  {
    oper = new DefSclOut<Type>;
  }
  else if(strcmp(activ, "CROSS") == 0)
  {
    oper = new CrossProduct<Type>();
  }
  else if(strcmp(activ, "LNCMB") == 0)
  {
    oper = new LineComb<Type>();
  }
  else if(strcmp(activ, "SCALR") == 0)
  {
    oper = new DotProduct<Type>();
  }
  else if(strcmp(activ, "NRML") == 0)
  {
    oper = new Normalize<Type>();
  }
  else if(strcmp(activ, "ASSV") == 0)
  {
    oper = new AssignVector<Type>();
  }
  else if(strcmp(activ, "ASSS") == 0)
  {
    oper = new AssignScalar<Type>();
  }
  else if(strcmp(activ, "ADDS") == 0)
  {
    oper = new AddScalar<Type>();
  }
  else if(strcmp(activ, "SUBS") == 0)
  {
    oper = new SubScalar<Type>();
  }
  else if(strcmp(activ, "ADDV") == 0)
  {
    oper = new AddVector<Type>();
  }
  else if(strcmp(activ, "SUBV") == 0)
  {
    oper = new SubVector<Type>();
  }
  else if(strcmp(activ, "MULS") == 0)
  {
    oper = new MulScalar<Type>();
  }
  else if(strcmp(activ, "DIVS") == 0)
  {
    oper = new DivScalar<Type>();
  }
  if(oper == NULL)
  {
    Clean();
    return false;
  }
  std::vector<oneParam<Type> >& signature = oper->getFields();
  for(unsigned i = 0; i < signature.size(); i++)
  {
    ParseCurrentLexem(pLine, activ);
    if(signature[i].vector)
      *signature[i].param = ParseVector(activ, signature[i].input);
    else
      *signature[i].param = ParseScalar(activ, signature[i].input, signature[i].constant);
    if(!signature[i].optional && signature[i].param == NULL)
    {
      Clean();
      delete oper;
      return false;
    }
  }
  m_operators.push_back(oper);
  if(!oper->postRead())
  {
    Clean();
    return false;
  }
  return true;
}


//----------------------------------------------------------------------------
template <class Type>
bool VecManVM<Type>::ReadFromFile(const char *filename)
//----------------------------------------------------------------------------
{
  FILE                            *fp;
  fp = fopen(filename, "rt");
  if(fp == NULL)
  {
    return false;
  }

  int const maxStrLen = 1000;
  char      sLine[maxStrLen];
  char      *pRet;

  while(true)
  {
    pRet = fgets(sLine, maxStrLen, fp);
    if(pRet == NULL)
      break;
    if(!ProcessString(pRet))
      break;
  }

  fclose(fp);
  return true;
}

#endif