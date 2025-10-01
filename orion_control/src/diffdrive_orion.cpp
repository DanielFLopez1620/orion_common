#include "orion_control/diffdrive_orion.hpp"

namespace orion_control
{
    hardware_interface::CallbackReturn DiffDriveOrion::on_init(
        const hardware_interface::HardwareComponentInterfaceParams& params)
    {
        this->left_enc_ = std::make_shared<std_msgs::msg::Int64>();
        this->right_enc_ = std::make_shared<std_msgs::msg::Int64>();
        this->cmd_speed_ = std::make_shared<std_msgs::msg::Int64MultiArray>();

        if(hardware_interface::SystemInterface::on_init(params) != hardware_interface::CallbackReturn::SUCCESS)
        {
            return hardware_interface::CallbackReturn::ERROR;
        }

        auto info_ = params.hardware_info;

        RCLCPP_INFO(this->logger_, "Diff: Begin [on_init]...");

        this->config_.left_wheel_name = info_.hardware_parameters.at("left_wheel_name");
        this->config_.right_wheel_name = info_.hardware_parameters.at("right_wheel_name");
        this->config_.enc_tics_per_rev = std::stoi(info_.hardware_parameters.at("enc_tics_per_rev"));

        this->left_wheel_.Setup(this->config_.left_wheel_name, this->config_.enc_tics_per_rev);
        this->right_wheel_.Setup(this->config_.right_wheel_name, this->config_.enc_tics_per_rev);

        if (auto locked_executor = params.executor.lock())
        {
            this->bridge_node_ = std::make_shared<OrionDiffBridgeNode>(
                info_.name,
                this->left_enc_,
                this->right_enc_,
                this->cmd_speed_);

            locked_executor->add_node(this->bridge_node_->get_node_base_interface());

            RCLCPP_INFO(this->logger_, "Diff: Created shared bridge node [on_init]...");
        }
        else
        {
            RCLCPP_ERROR(this->logger_, "Diff: Failed to lock executor for shared node [on_init]...");
            return hardware_interface::CallbackReturn::ERROR;
        }

        RCLCPP_INFO(this->logger_, "Diff: End [on_init]...");
        return hardware_interface::CallbackReturn::SUCCESS;
    }


    hardware_interface::CallbackReturn DiffDriveOrion::on_configure(const rclcpp_lifecycle::State&)
    {
        RCLCPP_INFO(this->logger_, "Diff: Begin [on_configure]...");
        RCLCPP_INFO(this->logger_, "Diff: End [on_configure]...");
        return hardware_interface::CallbackReturn::SUCCESS;
    }

    std::vector<hardware_interface::StateInterface> DiffDriveOrion::export_state_interfaces()
    {
        RCLCPP_INFO(this->logger_, "Diff: Begin [export_state_interfaces]...");
        std::vector<hardware_interface::StateInterface> state_interfaces;

        state_interfaces.emplace_back(
            hardware_interface::StateInterface(this->left_wheel_.name_,
                hardware_interface::HW_IF_VELOCITY, &this->left_wheel_.vel_));
        state_interfaces.emplace_back(
            hardware_interface::StateInterface(this->left_wheel_.name_,
                hardware_interface::HW_IF_POSITION, &this->left_wheel_.pos_));
        state_interfaces.emplace_back(
            hardware_interface::StateInterface(this->right_wheel_.name_,
                hardware_interface::HW_IF_VELOCITY, &this->right_wheel_.vel_));
        state_interfaces.emplace_back(
            hardware_interface::StateInterface(this->right_wheel_.name_,
                hardware_interface::HW_IF_POSITION, &this->right_wheel_.pos_));

        RCLCPP_INFO(this->logger_, "Diff: End [export_state_interfaces]...");

        return state_interfaces;
    }

    std::vector<hardware_interface::CommandInterface> DiffDriveOrion::export_command_interfaces()
    {
        RCLCPP_INFO(this->logger_, "Diff: Begin [export_command_interfaces]...");

        std::vector <hardware_interface::CommandInterface> command_interfaces;

        command_interfaces.emplace_back(
            hardware_interface::CommandInterface(this->left_wheel_.name_,
                hardware_interface::HW_IF_VELOCITY, &this->left_wheel_.cmd_));
        command_interfaces.emplace_back(
            hardware_interface::CommandInterface(this->right_wheel_.name_,
                hardware_interface::HW_IF_VELOCITY, &this->right_wheel_.cmd_));

        RCLCPP_INFO(this->logger_, "Diff: End [export_command_interfaces]...");

        return command_interfaces;
    }

    hardware_interface::CallbackReturn DiffDriveOrion::on_activate(const rclcpp_lifecycle::State&)
    {
        RCLCPP_INFO(this->logger_, "Diff: Begin [on_activate]...");
        RCLCPP_INFO(this->logger_, "Diff: End [on_activate]...");
        return hardware_interface::CallbackReturn::SUCCESS;
    }

    hardware_interface::CallbackReturn DiffDriveOrion::on_deactivate(const rclcpp_lifecycle::State&)
    {
        RCLCPP_INFO(this->logger_, "Diff: Begin [on_deactivate]...");
        RCLCPP_INFO(this->logger_, "Diff: End [on_deactivate]...");
        return hardware_interface::CallbackReturn::SUCCESS;
    }

    hardware_interface::return_type DiffDriveOrion::read(const rclcpp::Time&, const rclcpp::Duration& period)
    {
        RCLCPP_DEBUG(this->logger_, "Diff: Begin [read]...");

        const double d_t = period.seconds();

        this->left_wheel_.enc_ = this->left_enc_->data;
        this->right_wheel_.enc_ = this->right_enc_->data;

        const double left_pos_prev = this->left_wheel_.pos_;
        this->left_wheel_.pos_ = this->left_wheel_.Angle();
        this->left_wheel_.vel_ = (this->left_wheel_.pos_ - left_pos_prev) / d_t;

        const double right_pos_prev = this->right_wheel_.pos_;
        this->right_wheel_.pos_ = this->right_wheel_.Angle();
        this->right_wheel_.vel_ = (this->right_wheel_.pos_ - right_pos_prev) / d_t;

        RCLCPP_DEBUG(this->logger_, "Diff: End [read]...");

        return hardware_interface::return_type::OK;
    }

    hardware_interface::return_type DiffDriveOrion::write(const rclcpp::Time&, const rclcpp::Duration&)
    {
        RCLCPP_DEBUG(this->logger_, "Diff: Begin [write]...");

        const int left_cmd = static_cast<int>(this->left_wheel_.cmd_ / this->left_wheel_.rads_per_tick_);
        const int right_cmd = static_cast<int>(this->right_wheel_.cmd_ / this->right_wheel_.rads_per_tick_);

        this->cmd_speed_->data = {left_cmd, right_cmd};

        RCLCPP_DEBUG(this->logger_, "Diff: Begin [write]...");
        return hardware_interface::return_type::OK;
    }
}

#include <pluginlib/class_list_macros.hpp>
PLUGINLIB_EXPORT_CLASS(orion_control::DiffDriveOrion, hardware_interface::SystemInterface)