//
//  bvh.h
//  Ragnarok
//
//  Created by 莫少华 on 2022/2/3.
//

#ifndef bvh_h
#define bvh_h

#include <queue>
#include <stack>
#include <algorithm>

#include "common.h"
#include "hittable.h"

struct bvh_node
{
    aabb bounding;
    bvh_node *left, *right;
	uint32_t start = 0, end = 0;

	inline bool is_leaf() const 
	{
		return left == nullptr && right == nullptr;
	}
};

// triangle used in building bvh
struct building_node
{
	shared_ptr<hittable> shape_ptr;
	shared_ptr<aabb> bound_ptr;
	vec3 center;
};

class bvh : public hittable
{
public:
    bvh(const std::vector<shared_ptr<hittable>>& src_objects) 
    {
        //aabb_ptr = make_shared<aabb>();

        int obj_count = src_objects.size();
		std::vector<building_node> build_nodes(obj_count);
		for (uint32_t i = 0; i < obj_count; ++i)
		{
            auto& build_node = build_nodes[i];
			build_node.shape_ptr = src_objects[i];
			build_node.bound_ptr = src_objects[i]->bounding_box();
			build_node.center = (build_node.bound_ptr->max() + build_node.bound_ptr->min()) * 0.5;
            //aabb_ptr->extand(build_shape.bound_ptr);
		}

		const uint32_t leaf_capacity = 8;
		build_bvh(build_nodes, leaf_capacity);

		shape_ptrs.reserve(obj_count);
		for (auto bs : build_nodes)
			shape_ptrs.push_back(bs.shape_ptr);
    }

	virtual bool hit_fast(const ray& r, fType t_min, fType t_max) const override;
	virtual bool hit(const ray& r, fType t_min, fType t_max, hit_cache& cache) const override;

private:
	std::vector<shared_ptr<hittable>> shape_ptrs;
	bvh_node* bvh_root = nullptr;

	void build_bvh(std::vector<building_node>& shapes, uint32_t leaf_capacity)
	{
		struct BuildingTask
		{
			bvh_node** fillback;
			uint32_t start, end;
			uint32_t depth;
		};

		std::queue<BuildingTask> tasks;
		tasks.push({ &bvh_root, 0, (uint32_t)shapes.size(), 0 });

		while (!tasks.empty())
		{
			BuildingTask task = tasks.front();
			tasks.pop();

			aabb all_bound, center_bound;
			for (int i = task.start; i < task.end; i++)
			{
				auto& tri = shapes[i];
				all_bound.extand(tri.bound_ptr);
				center_bound.extand(tri.center);
			}

			// construct leaf node when triangle count is sufficiently low
			uint32_t tri_num = task.end - task.start;
			if (tri_num <= leaf_capacity)
			{
				//TODO use mem pool
				auto leaf = new bvh_node();

				leaf->bounding = all_bound;
				leaf->left = nullptr;
				leaf->right = nullptr;
				leaf->start = task.start;
				leaf->end = task.end;

				*task.fillback = leaf;
				continue;
			}

			// split alone the longest axis
			vec3 center_bound_size = center_bound.size();
			int split_axis = center_bound_size.x > center_bound_size.y ?
				(center_bound_size.x > center_bound_size.z ? 0 : 2) :
				(center_bound_size.y > center_bound_size.z ? 1 : 2);

			uint32_t middle;
			if (task.depth < 64)
			{
				fType split_value = 0.5 * (center_bound.maximum[split_axis] + center_bound.minimum[split_axis]);
				middle = task.start;

				for (uint32_t i = task.start; i < task.end; i++)
				{
					if (shapes[i].center[split_axis] < split_value)
						std::swap(shapes[i], shapes[middle++]);
				}

				//bad split
				if (middle == task.start || middle == task.end)
					middle = task.start + tri_num / 2;
			}
			else
			{
				//divide with triangle count when depth is too large
				std::sort(shapes.begin() + task.start, shapes.begin() + task.end,
					[axis = split_axis](const building_node& a, const building_node& b)
					{
						return a.center[axis] < b.center[axis];
					});
				middle = task.start + tri_num / 2;
			}

			//TODO use mem pool
			auto interior = new bvh_node();
			interior->bounding = all_bound;
			interior->left = nullptr;
			interior->right = nullptr;
			interior->start = 0;
			interior->end = 0;

			*task.fillback = interior;

			tasks.push({ &interior->left, task.start, middle, task.depth + 1 });
			tasks.push({ &interior->right, middle, task.end, task.depth + 1 });
		}
	}
};

bool bvh::hit_fast(const ray& r, fType t_min, fType t_max) const
{
	thread_local std::stack<bvh_node*> node_stack;
	node_stack.push(bvh_root);

	while (!node_stack.empty())
	{
		bvh_node* node = node_stack.top();
		node_stack.pop();

		if (!node || !node->bounding.hit(r, t_min, t_max))
			continue;

		if (node->is_leaf())
		{
			for (uint32_t i = node->start; i < node->end; i++)
			{
				shared_ptr<hittable> shape = shape_ptrs[i];
				if (shape && shape->hit_fast(r, t_min, t_max))
					return true;
			}
		}
		else
		{
			bvh_node* left = node->left;
			if (left && left->bounding.hit(r, t_min, t_max))
				node_stack.push(left);

			bvh_node* right = node->right;
			if (right && right->bounding.hit(r, t_min, t_max))
				node_stack.push(right);
		}
	}

	return false;
}

bool bvh::hit(const ray& r, fType t_min, fType t_max, hit_cache& cache) const
{
	hit_cache temp_cache;
	thread_local std::stack<bvh_node*> node_stack;
	node_stack.push(bvh_root);

	while (!node_stack.empty())
	{
		bvh_node* node = node_stack.top();
		node_stack.pop();

		if (!node || !node->bounding.hit(r, t_min, t_max))
			continue;

		if (node->is_leaf())
		{
			for (uint32_t i = node->start; i < node->end; i++)
			{
				shared_ptr<hittable> shape = shape_ptrs[i];
				if (shape && shape->hit(r, t_min, t_max, temp_cache))
				{
					cache = temp_cache;
					t_max = temp_cache.t;
				}
			}
		}
		else
		{
			bvh_node* left = node->left;
			if (left && left->bounding.hit(r, t_min, t_max))
				node_stack.push(left);

			bvh_node* right = node->right;
			if (right && right->bounding.hit(r, t_min, t_max))
				node_stack.push(right);
		}
	}

	if(cache.t < t_min || cache.t > t_max)
		return false;

	return true;
}

#endif /* bvh_h */
