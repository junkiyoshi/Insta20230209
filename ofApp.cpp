#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {

	ofSetFrameRate(60);
	ofSetWindowTitle("openFrameworks");

	ofBackground(255);
	ofEnableDepthTest();

	this->radius = 30;

	this->box2d.init();
	this->box2d.setGravity(0, 0);
	this->box2d.createBounds(30, 30, ofGetWidth() - 60, ofGetHeight() - 60);
	this->box2d.setFPS(60);
	this->box2d.registerGrabbing();

	for (int i = 0; i < 40; i++) {

		auto circle = make_shared<ofxBox2dCircle>();
		circle->setPhysics(0.5, 0.63, 0.1);
		circle->setup(this->box2d.getWorld(), ofRandom(30, ofGetWidth() - 30), ofRandom(30, ofGetHeight() - 30), this->radius);
		this->circles.push_back(circle);
		vector<glm::vec2> log;
		this->log_list.push_back(log);
	}
}

//--------------------------------------------------------------
void ofApp::update() {

	ofSeedRandom(39);

	for (int i = 0; i < this->circles.size(); i++) {

		for (int j = i + 1; j < this->circles.size(); j++) {

			float distance = this->circles[i]->getPosition().distance(this->circles[j]->getPosition());
			if (distance < this->radius * 4) {

				this->circles[i]->addForce(this->circles[i]->getPosition() - this->circles[j]->getPosition(), ofMap(distance, this->radius, this->radius * 4, 1.6, 0.004));
				this->circles[j]->addForce(this->circles[j]->getPosition() - this->circles[i]->getPosition(), ofMap(distance, this->radius, this->radius * 4, 1.6, 0.004));
			}
		}
	}

	for (int i = 0; i < this->circles.size(); i++) {

		this->log_list[i].push_back(this->circles[i]->getPosition());
		while (this->log_list[i].size() > 25) {

			this->log_list[i].erase(this->log_list[i].begin());
		}
	}

	this->box2d.update();
}

//--------------------------------------------------------------
void ofApp::draw() {

	auto threshold = 110;
	int head_size = 8;

	ofColor color;
	ofSetLineWidth(1);

	vector<glm::vec2> locations;
	for (int i = 0; i < this->circles.size(); i++) {

		glm::vec2 location = this->circles[i]->getPosition();
		locations.push_back(location);

		auto hue = ofMap(i, 0, this->circles.size(), 0, 255);
		color.setHsb(hue, 255, 255);
		ofSetColor(color);

		if (this->log_list[i].size() < 2) continue;

		ofMesh mesh;
		vector<glm::vec3> right, left, frame;
		glm::vec3 last_location;
		float last_theta;

		for (int k = 0; k < this->log_list[i].size() - 1; k++) {

			auto location = glm::vec3(this->log_list[i][k], 0);
			auto next = glm::vec3(this->log_list[i][k + 1], 0);

			auto direction = next - location;
			auto theta = atan2(direction.y, direction.x);

			right.push_back(location + glm::vec3(ofMap(k, 0, this->log_list[i].size(), 0.1, head_size) * cos(theta + PI * 0.5), ofMap(k, 0, this->log_list[i].size(), 0.1, head_size) * sin(theta + PI * 0.5), 0));
			left.push_back(location + glm::vec3(ofMap(k, 0, this->log_list[i].size(), 0.1, head_size) * cos(theta - PI * 0.5), ofMap(k, 0, this->log_list[i].size(), 0.1, head_size) * sin(theta - PI * 0.5), 0));

			last_location = location;
			last_theta = theta;
		}

		for (int k = 0; k < right.size(); k++) {

			mesh.addVertex(left[k]);
			mesh.addVertex(right[k]);
		}

		for (int k = 0; k < mesh.getNumVertices() - 2; k += 2) {

			mesh.addIndex(k + 0); mesh.addIndex(k + 1); mesh.addIndex(k + 3);
			mesh.addIndex(k + 0); mesh.addIndex(k + 2); mesh.addIndex(k + 3);
		}

		auto tmp_head_size = ofMap(log_list[i].size() - 2, 0, log_list[i].size(), 0.1, head_size);
		mesh.addVertex(last_location);

		int index = mesh.getNumVertices();
		for (auto theta = last_theta - PI * 0.5; theta <= last_theta + PI * 0.5; theta += PI / 20) {

			mesh.addVertex(last_location + glm::vec3(tmp_head_size * cos(theta), tmp_head_size * sin(theta), 0));
		}

		for (int i = index; i < mesh.getNumVertices() - 1; i++) {

			mesh.addIndex(index); mesh.addIndex(i + 0); mesh.addIndex(i + 1);
		}

		mesh.draw();

		ofFill();
		ofDrawCircle(this->log_list[i].back(), 8);
	}

	ofSetColor(0);
	ofSetLineWidth(1);

	for (auto& location : locations) {

		int word_index = 0;
		for (auto& other : locations) {

			if (location == other) { continue; }

			auto distance = glm::distance(location, other);
			if (distance < threshold) {

				auto direction_rad = std::atan2(other.y - location.y, other.x - location.x);
				auto direction = direction_rad * RAD_TO_DEG;
				auto width = ofMap(distance, 0, threshold, 360, 0);

				ofPushMatrix();
				ofTranslate(location);

				ofNoFill();
				ofBeginShape();
				for (auto deg = direction - width * 0.5; deg <= direction + width * 0.5; deg++) {

					ofVertex(radius * cos(deg * DEG_TO_RAD), radius * sin(deg * DEG_TO_RAD));
				}
				ofEndShape();

				ofDrawLine(glm::vec2(radius * cos(direction * DEG_TO_RAD), radius * sin(direction * DEG_TO_RAD)),
					(other - location) + glm::vec2(radius * cos((180 + direction) * DEG_TO_RAD), radius * sin((180 + direction) * DEG_TO_RAD)));

				ofPopMatrix();
			}
		}
	}
}

//--------------------------------------------------------------
int main() {

	ofSetupOpenGL(720, 720, OF_WINDOW);
	ofRunApp(new ofApp());
}