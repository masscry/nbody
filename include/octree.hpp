#pragma once
#ifndef __OCTREE_HEADER__
#define __OCTREE_HEADER__

#include <vector>
#include <stdexcept>

union aabb_t {
	struct {
		glm::vec3 tl;
		glm::vec3 br;
	};
	glm::vec3 v[2];

	float SqSize() const {
		glm::vec3 dlt = this->tl - this->br;
		return dlt.x* dlt.x + dlt.y * dlt.y + dlt.z * dlt.z;
	}

	float Size() const {
		return glm::length(this->tl - this->br);
	}

};

const int OCLEAF_TOTAL = 8;

const float G = 6.6742867e-5f;

template<typename T>
class ocleaf_t {
private:
	aabb_t aabb;
	ocleaf_t<T>* leaves;
	glm::vec3 bcenter;
	int total;

	constexpr aabb_t ChoosePoint(aabb_t aabb, glm::vec3 sz, glm::ivec3 ch)
	{
		return aabb_t{
		  glm::vec3{ ch.x ? aabb.tl.x : sz.x, ch.y ? aabb.tl.y : sz.y, ch.z ? aabb.tl.z : sz.z },
		  glm::vec3{ ch.x ? sz.x : aabb.br.x, ch.y ? sz.y : aabb.br.y, ch.z ? sz.z : aabb.br.z }
		};
	}

	void MakeLeaves() {
		if (this->leaves == nullptr) {
			glm::vec3 size = (aabb.tl + aabb.br) / 2.0f;
			this->leaves = new ocleaf_t<T>[OCLEAF_TOTAL];

			this->leaves[0].aabb = ChoosePoint(this->aabb, size, glm::ivec3(0, 0, 0));
			this->leaves[1].aabb = ChoosePoint(this->aabb, size, glm::ivec3(0, 1, 0));
			this->leaves[2].aabb = ChoosePoint(this->aabb, size, glm::ivec3(1, 1, 0));
			this->leaves[3].aabb = ChoosePoint(this->aabb, size, glm::ivec3(1, 0, 0));
			this->leaves[4].aabb = ChoosePoint(this->aabb, size, glm::ivec3(0, 0, 1));
			this->leaves[5].aabb = ChoosePoint(this->aabb, size, glm::ivec3(0, 1, 1));
			this->leaves[6].aabb = ChoosePoint(this->aabb, size, glm::ivec3(1, 1, 1));
			this->leaves[7].aabb = ChoosePoint(this->aabb, size, glm::ivec3(1, 0, 1));
		}
	}

public:

	glm::vec3 GetForceOnPoint(glm::vec3 point) const {
		if (this->total == 0) {
			return glm::vec3(0.0f);
		}

		glm::vec3 dlt = this->Barycenter() - point;
		float sqlen = dlt.x*dlt.x + dlt.y*dlt.y + dlt.z*dlt.z;

		if (sqlen == 0.0f) {
			return glm::vec3(0.0f);
		}

		if ((this->leaves == 0) || ((aabb.SqSize() / sqlen) < 10.0f)) {
			dlt /= std::sqrt(sqlen);
			sqlen = (sqlen < 1.0f) ? 1.0f : sqlen;
			return dlt * this->Mass()* G / sqlen;
		}
		else
		{
			return this->leaves[0].GetForceOnPoint(point) + this->leaves[1].GetForceOnPoint(point)
				+ this->leaves[2].GetForceOnPoint(point) + this->leaves[3].GetForceOnPoint(point)
				+ this->leaves[4].GetForceOnPoint(point) + this->leaves[5].GetForceOnPoint(point)
				+ this->leaves[6].GetForceOnPoint(point) + this->leaves[7].GetForceOnPoint(point);
		}
	}

	glm::vec3 Barycenter() const {
		return this->bcenter / ((float)total);
	}

	float Mass() const {
		return (float)total;
	}

	std::vector<glm::vec3> BuildBox() const {
		if (this->total == 0) {
			return std::vector<glm::vec3>();
		}

		std::vector<glm::vec3> result{
		  { aabb.tl.x, aabb.tl.y, aabb.tl.z }, { aabb.br.x, aabb.tl.y, aabb.tl.z },
		  { aabb.br.x, aabb.tl.y, aabb.tl.z }, { aabb.br.x, aabb.br.y, aabb.tl.z },
		  { aabb.br.x, aabb.br.y, aabb.tl.z }, { aabb.tl.x, aabb.br.y, aabb.tl.z },
		  { aabb.tl.x, aabb.br.y, aabb.tl.z }, { aabb.tl.x, aabb.tl.y, aabb.tl.z },

		  { aabb.tl.x, aabb.tl.y, aabb.tl.z }, { aabb.tl.x, aabb.tl.y, aabb.br.z },
		  { aabb.br.x, aabb.tl.y, aabb.tl.z }, { aabb.br.x, aabb.tl.y, aabb.br.z },
		  { aabb.br.x, aabb.br.y, aabb.tl.z }, { aabb.br.x, aabb.br.y, aabb.br.z },
		  { aabb.tl.x, aabb.br.y, aabb.tl.z }, { aabb.tl.x, aabb.br.y, aabb.br.z },

		  { aabb.tl.x, aabb.tl.y, aabb.br.z }, { aabb.br.x, aabb.tl.y, aabb.br.z },
		  { aabb.br.x, aabb.tl.y, aabb.br.z }, { aabb.br.x, aabb.br.y, aabb.br.z },
		  { aabb.br.x, aabb.br.y, aabb.br.z }, { aabb.tl.x, aabb.br.y, aabb.br.z },
		  { aabb.tl.x, aabb.br.y, aabb.br.z }, { aabb.tl.x, aabb.tl.y, aabb.br.z }
		};

		if (this->leaves != nullptr) {
			for (int i = 0; i < OCLEAF_TOTAL; ++i) {
				std::vector<glm::vec3> tmp;
				tmp = this->leaves[i].BuildBox();
				result.insert(result.end(), tmp.begin(), tmp.end());
			}
		}
		return result;
	}

	ocleaf_t() :aabb(), leaves(nullptr), bcenter(), total(0) {
		;
	}

	ocleaf_t(aabb_t aabb) :aabb(aabb), leaves(nullptr), bcenter(), total(0) {
		;
	}

	ocleaf_t(const ocleaf_t & copy) :aabb(copy.aabb), leaves(nullptr), bcenter(copy.bcenter), total(copy.total) {
		if (copy.leaves != nullptr) {
			this->leaves = new ocleaf_t<T>[OCLEAF_TOTAL];
			for (int i = 0; i < OCLEAF_TOTAL; ++i) {
				this->leaves[i] = copy.leaves[i];
			}
		}
	}

	ocleaf_t(ocleaf_t && move) :aabb(move.aabb), leaves(move.leaves), bcenter(move.bcenter), total(move.total) {
		move.leaves = nullptr;
	}

	ocleaf_t<T>& operator = (const ocleaf_t<T> & copy) {
		if (this != &copy) {
			this->aabb = copy.aabb;
			if (copy.leaves != nullptr) {
				if (this->leaves == nullptr) {
					this->leaves = new ocleaf_t<T>[OCLEAF_TOTAL];
				}
				for (int i = 0; i < OCLEAF_TOTAL; ++i) {
					this->leaves[i] = copy.leaves[i];
				}
				this->bcenter = copy.bcenter;
				this->total = copy.total;
			}
			else {
				delete[] this->leaves;
				this->leaves = nullptr;
			}
		}
		return *this;
	}

	ocleaf_t<T>& operator = (ocleaf_t<T> && move) {
		if (this != &move) {
			this->aabb = move.aabb;
			delete[] this->leaves;
			this->leaves = move.leaves;
			this->bcenter = move.bcenter;
			this->total = move.total;
			move.leaves = nullptr;
		}
		return *this;
	}

	~ocleaf_t() {
		delete[] this->leaves;
	}

	bool Inside(glm::vec3 point) {
		return (aabb.br.x <= point.x) && (point.x <= aabb.tl.x)
			&& (aabb.br.y <= point.y) && (point.y <= aabb.tl.y)
			&& (aabb.br.z <= point.z) && (point.z <= aabb.tl.z);
	}

	void Push(glm::vec3 point) {
		if (!this->Inside(point)) {
			throw std::runtime_error("Point is Out of AABB");
		}

		if (this->total == 0)
		{
			this->bcenter = point;
		}
		else
		{
			this->bcenter += point;
			this->MakeLeaves();
			for (int i = 0; i < OCLEAF_TOTAL; ++i)
			{
				if (this->leaves[i].Inside(point)) {
					this->leaves[i].Push(point);
					break;
				}
			}
		}
		++this->total;
	}

};

template<typename T>
class octree_t {
private:
	ocleaf_t<T>* head;

public:

	void Push(glm::vec3 point) {
		this->head->Push(point);
	}

	const ocleaf_t<T>& Root() const {
		return *this->head;
	}

	octree_t(aabb_t aabb) :head(nullptr) {
		this->head = new ocleaf_t<T>(aabb);
	}

	octree_t(const octree_t& copy) :head(nullptr) {
		this->head = new ocleaf_t<T>(copy);
	}

	octree_t(octree_t<T>&& move) :head(nullptr) {
		this->head = move.head;
		move.head = nullptr;
	}

	octree_t<T>& operator = (const octree_t<T>& copy) {
		if (this != &copy) {
			*(this->head) = *copy.head;
		}
		return *this;
	}

	octree_t<T>& operator = (octree_t<T>&& move) {
		if (this != &move) {
			*(this->head) = std::move(*move.head);
		}
		return *this;
	}

	~octree_t() {
		delete this->head;
	}

};

#endif /* __OCTREE_HEADER__ */