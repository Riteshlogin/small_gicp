// SPDX-FileCopyrightText: Copyright 2024 Kenji Koide
// SPDX-License-Identifier: MIT

/// @brief Basic point cloud registration example with PCL interfaces
#include <small_gicp/pcl/pcl_point.hpp>
#include <small_gicp/pcl/pcl_point_traits.hpp>
#include <small_gicp/pcl/pcl_registration.hpp>
#include <small_gicp/util/downsampling_omp.hpp>
#include <small_gicp/benchmark/read_points.hpp>

using namespace small_gicp;

/// @brief Example of using RegistrationPCL that can be used as a drop-in replacement for pcl::GeneralizedIterativeClosestPoint.
void example1(const pcl::PointCloud<pcl::PointXYZ>::ConstPtr& raw_target, const pcl::PointCloud<pcl::PointXYZ>::ConstPtr& raw_source, double ds_factor,
  Eigen::Isometry3d& final_transform) {
  // small_gicp::voxelgrid_downsampling can directly operate on pcl::PointCloud.
  pcl::PointCloud<pcl::PointXYZ>::Ptr target = voxelgrid_sampling_omp(*raw_target, ds_factor);
  pcl::PointCloud<pcl::PointXYZ>::Ptr source = voxelgrid_sampling_omp(*raw_source, 0.95);

  // RegistrationPCL is derived from pcl::Registration and has mostly the same interface as pcl::GeneralizedIterativeClosestPoint.
  RegistrationPCL<pcl::PointXYZ, pcl::PointXYZ> reg;
  reg.setNumThreads(4);
  reg.setCorrespondenceRandomness(20);
  reg.setMaxCorrespondenceDistance(1.0);
  reg.setVoxelResolution(1.0);
  reg.setRegistrationType("VGICP");  // or "GICP" (default = "GICP")

  // Set input point clouds.
  reg.setInputTarget(target);
  reg.setInputSource(source);

  // Align point clouds.
  auto aligned = pcl::make_shared<pcl::PointCloud<pcl::PointXYZ>>();
  reg.align(*aligned);
  final_transform = reg.getFinalTransformation().cast<double>();
  std::cout << "--- T_target_source ---" << std::endl << reg.getFinalTransformation() << std::endl;
  std::cout << "--- H ---" << std::endl << reg.getFinalHessian() << std::endl;

  // Swap source and target and align again.
  // This is useful when you want to re-use preprocessed point clouds for successive registrations (e.g., odometry estimation).
  reg.swapSourceAndTarget();
  reg.align(*aligned);

  //std::cout << "--- T_target_source ---" << std::endl << reg.getFinalTransformation().inverse() << std::endl;
}

int main(int argc, char** argv) {
  std::vector<Eigen::Vector4f> source_points = read_ply("data/source.ply");
  std::vector<Eigen::Vector4f> target_points = read_ply("data/translated_source.ply");
  if (target_points.empty() || source_points.empty()) {
    std::cerr << "error: failed to read points from data/(target|source).ply" << std::endl;
    return 1;
  }

  const auto convert_to_pcl = [](const std::vector<Eigen::Vector4f>& raw_points) {
    auto points = pcl::make_shared<pcl::PointCloud<pcl::PointXYZ>>();
    points->resize(raw_points.size());
    for (size_t i = 0; i < raw_points.size(); i++) {
      points->at(i).getVector4fMap() = raw_points[i];
    }
    return points;
  };

  pcl::PointCloud<pcl::PointXYZ>::Ptr raw_target = convert_to_pcl(target_points);
  pcl::PointCloud<pcl::PointXYZ>::Ptr raw_source = convert_to_pcl(source_points);
  Eigen::Isometry3d result;

  double truth_x = 0.5;
  double truth_y = 0.5;
  double truth_z = 0.5;
  std::fstream errorFile;
  errorFile.open("errorAnalysis.csv", std::ios::out);
	for(double i = 0.3; i < 0.9; i = i + 0.1)
	{
  	example1(raw_target, raw_source, i, result);

    double error_x = std::abs(result(0, 3) - truth_x);
    double error_y = std::abs(result(1, 3) - truth_y);
    double error_z = std::abs(result(2, 3) - truth_z);
  
    errorFile << i << "," << error_x << "," << error_y << "," << error_z << "\n";

    printf("\nerrors are is %lf, %lf, %lf\n", error_x, error_y, error_z);

	}
  return 0;
}
