#pragma once
#include <Eigen/Dense>
#include "types.h"

Eigen::TMatrixX comp_delta(const Eigen::Ref<const Eigen::TMatrixX>& static_coef, int DELTAWINDOW);

