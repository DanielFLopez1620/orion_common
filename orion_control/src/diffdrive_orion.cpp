#include "orion_control/diffdrive_orion.hpp"

namespace orion_control
{
    hardware_interface::CallbackReturn DiffDriveOrion::on_init(
        const hardware_interface::HardwareComponentInterfaceParams& params)
    {

        if(hardware_interface::SystemInterface::on_init(params) != hardware_interface::CallbackReturn::SUCCESS)
        {
            return hardware_interface::CallbackReturn::ERROR;
        }

        auto info_ = params.hardware_info;

        this->left_enc_ = std::make_shared<std_msgs::msg::Int64>();
        this->right_enc_ = std::make_shared<std_msgs::msg::Int64>();
        this->cmd_speed_ = std::make_shared<std_msgs::msg::Int64MultiArray>();

        RCLCPP_INFO(this->logger_, "Diff: Begin [on_init]...");

        this->config_.left_wheel_name =
            info_.hardware_parameters.at("left_wheel_name");
        this->config_.right_wheel_name =
            info_.hardware_parameters.at("right_wheel_name");
        this->config_.enc_tics_per_rev =
            std::stoi(info_.hardware_parameters.at("enc_ticks_per_rev"));

        // Optional topic overrides — fall back to defaults if not specified in URDF
        auto read_param = [&](const std::string& key, const std::string& fallback) {
            auto it = info_.hardware_parameters.find(key);
            return (it != info_.hardware_parameters.end()) ? it->second : fallback;
        };
        this->config_.motor_cmd_topic =
            read_param("motor_cmd_topic", this->config_.motor_cmd_topic);
        this->config_.left_enc_topic =
            read_param("left_enc_topic", this->config_.left_enc_topic);
        this->config_.right_enc_topic =
            read_param("right_enc_topic", this->config_.right_enc_topic);

        this->left_wheel_.Setup(
            this->config_.left_wheel_name, this->config_.enc_tics_per_rev);
        this->right_wheel_.Setup(
            this->config_.right_wheel_name, this->config_.enc_tics_per_rev);

        // Lock executor to add the bridge node
        if (auto locked_executor = params.executor.lock())
        {
            this->bridge_node_ = std::make_shared<OrionDiffBridgeNode>(
                info_.name,
                this->config_.motor_cmd_topic,
                this->config_.left_enc_topic,
                this->config_.right_enc_topic,
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

    } // on_init()

    hardware_interface::CallbackReturn DiffDriveOrion::on_configure(const rclcpp_lifecycle::State&)
    {
        RCLCPP_INFO(this->logger_, "Diff: Begin [on_configure]...");
        RCLCPP_INFO(this->logger_, "Diff: End [on_configure]...");
        return hardware_interface::CallbackReturn::SUCCESS;

    } // on_configure()

    std::vector<hardware_interface::StateInterface> DiffDriveOrion::export_state_interfaces()
    {
        RCLCPP_INFO(this->logger_, "Diff: Begin [export_state_interfaces]...");
        std::vector<hardware_interface::StateInterface> state_interfaces;

        state_interfaces.emplace_back(
            hardware_interface::StateInterface(
                this->left_wheel_.name_,
                hardware_interface::HW_IF_VELOCITY,
                &this->left_wheel_.vel_
        ));

        state_interfaces.emplace_back(
            hardware_interface::StateInterface(
                this->left_wheel_.name_,
                hardware_interface::HW_IF_POSITION,
                &this->left_wheel_.pos_
        ));

        state_interfaces.emplace_back(
            hardware_interface::StateInterface(
                this->right_wheel_.name_,
                hardware_interface::HW_IF_VELOCITY,
                &this->right_wheel_.vel_
        ));

        state_interfaces.emplace_back(
            hardware_interface::StateInterface(
                this->right_wheel_.name_,
                hardware_interface::HW_IF_POSITION,
                &this->right_wheel_.pos_
        ));

        RCLCPP_INFO(this->logger_, "Diff: End [export_state_interfaces]...");

        return state_interfaces;

    } // export_state_interfaces()

    std::vector<hardware_interface::CommandInterface> DiffDriveOrion::export_command_interfaces()
    {
        RCLCPP_INFO(this->logger_, "Diff: Begin [export_command_interfaces]...");

        std::vector <hardware_interface::CommandInterface> command_interfaces;

        command_interfaces.emplace_back(
            hardware_interface::CommandInterface(
                this->left_wheel_.name_,
                hardware_interface::HW_IF_VELOCITY,
                &this->left_wheel_.cmd_
        ));

        command_interfaces.emplace_back(
            hardware_interface::CommandInterface(
                this->right_wheel_.name_,
                hardware_interface::HW_IF_VELOCITY,
                &this->right_wheel_.cmd_
        ));

        RCLCPP_INFO(this->logger_, "Diff: End [export_command_interfaces]...");

        return command_interfaces;

    } // export_command_interfaces()

    hardware_interface::CallbackReturn DiffDriveOrion::on_activate(
        const rclcpp_lifecycle::State&)
    {
        RCLCPP_INFO(this->logger_, "Diff: Begin [on_activate]...");
        if (!this->bridge_node_)
        {
            RCLCPP_ERROR(this->logger_, "Diff: Bridge node not initialized — cannot activate.");
            return hardware_interface::CallbackReturn::ERROR;
        }
        RCLCPP_INFO(this->logger_, "Diff: End [on_activate]...");
        return hardware_interface::CallbackReturn::SUCCESS;

    } // on_activate()

    hardware_interface::CallbackReturn DiffDriveOrion::on_deactivate(
        const rclcpp_lifecycle::State&)
    {
        RCLCPP_INFO(this->logger_, "Diff: Begin [on_deactivate]...");
        this->cmd_speed_->data = {0, 0};
        RCLCPP_INFO(this->logger_, "Diff: End [on_deactivate]...");
        return hardware_interface::CallbackReturn::SUCCESS;

    } // on_deactivate()

    hardware_interface::return_type DiffDriveOrion::read(
        const rclcpp::Time&, const rclcpp::Duration& period)
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

    hardware_interface::return_type DiffDriveOrion::write(
        const rclcpp::Time&, const rclcpp::Duration&)
    {
        RCLCPP_DEBUG(this->logger_, "Diff: Begin [write]...");

        const int left_cmd =
            static_cast<int>(this->left_wheel_.cmd_ / this->left_wheel_.rads_per_tick_);
        const int right_cmd =
            static_cast<int>(this->right_wheel_.cmd_ / this->right_wheel_.rads_per_tick_);

        this->cmd_speed_->data = {left_cmd, right_cmd};

        RCLCPP_DEBUG(this->logger_, "Diff: End [write]...");
        return hardware_interface::return_type::OK;
    }

} // orion_control


#include <pluginlib/class_list_macros.hpp>
PLUGINLIB_EXPORT_CLASS(orion_control::DiffDriveOrion, hardware_interface::SystemInterface)
