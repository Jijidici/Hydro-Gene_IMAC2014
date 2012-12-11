#ifndef __FREEFLY_CAMERA_HPP__
#define __FREEFLY_CAMERA_HPP__

#include <glm/glm.hpp>

namespace hydrogene{

class FreeFlyCamera{
	private:
		glm::vec3 m_Position;
		float m_fPhi;
		float m_fTheta;
		glm::vec3 m_FrontVector;
		glm::vec3 m_LeftVector;
		glm::vec3 m_UpVector;

		void computeDirectionVectors();

	public:
		FreeFlyCamera();
		void moveLeft(float const t);
		void moveFront(float const t);
		void rotateLeft(float degree);
		void rotateUp(float degree);
		glm::mat4 getViewMatrix() const;
};

}

#endif