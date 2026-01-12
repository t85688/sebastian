#include <QProcess>

#include "act_device_configuration.hpp"

#define AGENT_TYPE_SWITCH_SSH (1)
#define COMMAND_LINE_OUTPUT "cliOutput.txt"
#define COMMAND_LINE_TIMEOUT (15000)  ///< The timeout(ms, 15 second) of the CLI

static QString GetCommandLineOutputPath() {
  // COMMAND_LINE_OUTPUT
  QString tmpPath = qEnvironmentVariable("CHAMBERLAIN_COGSWORTH_TMP_ROOT", "tmp");
  return tmpPath + QDir::separator() + "devicemgr" + QDir::separator() + COMMAND_LINE_OUTPUT;
}

ACT_STATUS ActDeviceConfiguration::StartCommandLine(const ActProject &project, const QList<qint64> &dev_id_list,
                                                    const QString &command) {
  // Checking has the thread is running
  if (IsActStatusRunning(device_config_act_status_)) {
    qCritical() << __func__ << "Currently has the thread running.";
    return std::make_shared<ActStatusInternalError>("DeviceConfiguration");
  }

  // init ActDeviceConfiguration status
  progress_ = 0;
  stop_flag_ = false;
  device_config_act_status_ = std::make_shared<ActStatusBase>(ActStatusType::kStop, ActSeverity::kDebug);

  // New std::thread to start CommandLine
  try {
    // Check previous thread already join, because smart pointer reassign would remove the previous thread
    if ((device_config_thread_ != nullptr) && (device_config_thread_->joinable())) {
      device_config_thread_->join();
    }
    device_config_act_status_->SetStatus(ActStatusType::kRunning);
    device_config_thread_ =
        std::make_unique<std::thread>(&ActDeviceConfiguration::TriggerCommandLineForThread, this, std::cref(project),
                                      std::cref(dev_id_list), std::cref(command));

#ifdef _WIN32
    // Set the thread name
    std::wstring thread_name = L"TriggerCommandLineForThread";
    HRESULT hr = SetThreadDescription(device_config_thread_->native_handle(), thread_name.c_str());
    if (FAILED(hr)) {
      // Handle error
    }
#endif

    qDebug() << "Start CommandLine thread.";

  } catch (std::exception &e) {
    qCritical() << __func__ << "New std::thread(CommandLine) failed. Error:" << e.what();
    device_config_act_status_->SetStatus(ActStatusType::kFailed);
    return std::make_shared<ActStatusInternalError>("DeviceConfiguration");
  }

  return std::make_shared<ActProgressStatus>(ActStatusBase(*device_config_act_status_), progress_);
}

void ActDeviceConfiguration::TriggerCommandLineForThread(const ActProject &project, const QList<qint64> &dev_id_list,
                                                         const QString &command) {
  // Waiting for main thread return running flag to WS
  std::this_thread::yield();

  // Triggered the CommandLine and wait for the return, and update device_config_thread_.
  try {
    device_config_act_status_ = CommandLine(project, dev_id_list, command);

  } catch (std::exception &e) {
    qCritical() << __func__ << "New std::thread(interrupt) failed. Error:" << e.what();
    device_config_act_status_ = std::make_shared<ActStatusInternalError>("DeviceConfiguration");
  }
}

ACT_STATUS ActDeviceConfiguration::CommandLine(const ActProject &project, const QList<qint64> &dev_id_list,
                                               const QString &command) {
  ACT_STATUS_INIT();

  QList<qint64> sorted_dev_id_list = dev_id_list;
  // SortDeviceIdList(project, sorted_dev_id_list);

  UpdateProgress(10);

  QString cmd = command;
  cmd.replace("\r\n", "\\r\\n");

  int timeout = COMMAND_LINE_TIMEOUT * cmd.split("\\n").size();

  QString commandLineOutputPath = GetCommandLineOutputPath();

  for (qint64 &dev_id : sorted_dev_id_list) {
    if (stop_flag_) {  // stop flag
      return ACT_STATUS_STOP;
    }

    ActDevice dev;
    act_status = project.GetDeviceById(dev, dev_id);
    if (!IsActStatusSuccess(act_status)) {
      DeviceConfigurationErrorHandler(__func__, "Device not found in the project", dev);
      continue;
    }

    QProcess proc;
    QStringList string_list;
    string_list << "device" << "cli" << "send" << "-a" << QString("%1").arg(AGENT_TYPE_SWITCH_SSH) << "-i"
                << dev.GetIpv4().GetIpAddress() << "-p" << QString("%1").arg(dev.GetSSHPort()) << "-u"
                << dev.GetAccount().GetUsername() << "-o" << dev.GetAccount().GetPassword() << "-c" << cmd;
    proc.start(GetAfdFilePath(), string_list);
    if (!proc.waitForStarted()) {
      DeviceConfigurationErrorHandler(__func__, "Unable to start process", dev);
    } else if (!proc.waitForFinished(timeout)) {
      DeviceConfigurationErrorHandler(__func__, "Timeout to start process", dev);
      proc.kill();
      proc.waitForFinished();
      continue;
    }

    QFile file(commandLineOutputPath);
    if (!file.open(QIODevice::ReadOnly)) {
      QString error_msg = QString("Command line output file doesn't exists: %1").arg(commandLineOutputPath);
      DeviceConfigurationErrorHandler(__func__, error_msg, dev);
      continue;
    }

    // Read all contents at one time
    QString content = file.readAll();

    // Close the license file
    file.close();

    // qDebug() << "command_lind_output" << command_lind_output.ToString().toStdString().c_str();
    UpdateProgress(progress_ + (80 / sorted_dev_id_list.size()));

    // Add success result to result_queue_
    result_queue_.enqueue(ActDeviceConfigureResult(dev.GetId(), ActStatusType::kSuccess, progress_, content));
  }

  UpdateProgress(100);
  return ACT_STATUS_SUCCESS;
}