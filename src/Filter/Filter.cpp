#include <Filter/Filter.h>

// vector contains poses... that's it. prediction updates current pose
bool Filter::prediction_step(const nav_msgs::Odometry::ConstPtr& u_msg/*Vector u*/)
{
	Vector x_old = state_.x();
	Matrix Sx_old = state_.Sx();

	if(motion_model_.preprocess(x_old,u_msg))
	{
		Matrix Fx = motion_model_.Fx();
		Matrix Fu = motion_model_.Fu();
		Matrix Su = motion_model_.Su();

		Vector x_new = motion_model_.f();

		Matrix Sx_new = Fx*Sx_old*Fx.transpose() + Fu*Su*Fu.transpose();

		state_.update(x_new, Sx_new);

		return true;
	}
	else
	{
		return false; // odom was not init
	}
}

// measurements are points in some coordinate frame. expected is from past regions... correspondences through ransac.
// no matching? just transform according to poses and find the difference
bool Filter::correction_step(const sensor_msgs::PointCloud2::ConstPtr& z_msg/*Vector z*/)
{
	Vector x_old = state_.x();
	Matrix Sx_old = state_.Sx();

	int n = x_old.size();

	if(measurement_model_.preprocess(x_old, z_msg))
	{
		Vector dz = measurement_model_.dz();
		Matrix Hx = measurement_model_.Hx();
		Matrix Sz = measurement_model_.Sz();
		Matrix I = Matrix::Identity(n,n);

		Matrix K = Sx_old*Hx.transpose()*(Hx*Sx_old*Hx.transpose()+Sz).inverse();

		//printf("dz(%ld)\n",dz.size());
		//printf("Hx(%ld,%ld)\n",Hx.rows(),Hx.cols());
		//printf("Sz(%ld,%ld)\n",Sz.rows(),Sz.cols());
		//printf("I(%ld,%ld)\n",I.rows(),I.cols());
		//printf("K(%ld,%ld)\n",K.rows(),K.cols());

		Vector x_new = x_old + K*dz;
		Matrix Sx_new = (I-K*Hx)*Sx_old;

		state_.update(x_new, Sx_new);
	}
	else
	{
		return false; // could not find correspondences? dunno
	}
}
