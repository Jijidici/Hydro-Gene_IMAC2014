#ifndef __TRACKBALL_CAMERA_HPP__
#define __TRACKBALL_CAMERA_HPP__

#include <glm/glm.hpp>

namespace hydrogene{

class TrackBallCamera{
	public:
		TrackBallCamera();
		void moveFront(float distance);
		void rotateLeft(float degrees);
		void rotateUp(float degrees);
		glm::mat4 getViewMatrix();
		void setCamPos(float xDeg, float yDeg, float distance);
		glm::vec3 getCameraPosition() const;

	private:

		float m_fDistance;
		float m_fAngleX;
		float m_fAngleY;
		glm::vec3 m_position;
};

}

#endif
