#include "tools/MatrixStack.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stack>

// --> Constructor
MatrixStack::MatrixStack(){
	this->m_Stack.push(glm::mat4(1.f));
}

// --> Push
void MatrixStack::push(){
	m_Stack.push(m_Stack.top());
}

// --> Pop
void MatrixStack::pop(){
	m_Stack.pop();
}

// --> Multiplication
void MatrixStack::mult(const glm::mat4& m){
	m_Stack.top() = m_Stack.top()*m;
}

// --> Getter de tete
const glm::mat4 MatrixStack::top() const{
	return m_Stack.top();
}

// --> Setter en tete
void MatrixStack::set(const glm::mat4& m){
	m_Stack.top() = m;
}

// --> Scale
void MatrixStack::scale(const glm::vec3& s){
	m_Stack.top() = glm::scale(m_Stack.top(), s);
}

// --> Translate
void MatrixStack::translate(const glm::vec3& t){
	m_Stack.top() = glm::translate(m_Stack.top(), t);
}

// --> Rotate
void MatrixStack::rotate(float degrees, const glm::vec3& r){
	m_Stack.top() = glm::rotate(m_Stack.top(), degrees, r);
}

// --> Destructor
MatrixStack::~MatrixStack(){
	
}
