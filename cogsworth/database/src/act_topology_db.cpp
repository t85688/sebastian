#include <QImage>

#include "act_db.hpp"
#include "act_topology.hpp"

// Namespace names are all lower-case, with words separated by underscores.
// https://google.github.io/styleguide/cppguide.html#Namespace_Names
namespace act {
namespace database {
namespace topology {

ACT_STATUS Init() {
  ACT_STATUS_INIT();

  // kene+
  /*
  QDir dir(ACT_TOPOLOGY_DB_FOLDER);
  */
  QString topologyDbFolder = act::database::GetTopologyDbFolder();
  QDir dir(topologyDbFolder);
  // kene-
  if (dir.exists()) {
    return act_status;
  }

  // kene+
  /*
  if (!QDir().mkpath(ACT_TOPOLOGY_BASE_DB_FOLDER)) {
    qDebug() << "mkpath() failed:" << ACT_TOPOLOGY_BASE_DB_FOLDER;
  */
  QString topologyBaseDbFolder = act::database::GetTopologyBaseDbFolder();
  if (!QDir().mkpath(topologyBaseDbFolder)) {
    qDebug() << "mkpath() failed:" << topologyBaseDbFolder;
    // kene-
    return std::make_shared<ActStatusInternalError>("Database");
  }

  // kene+
  /*
  if (!QDir().mkpath(ACT_TOPOLOGY_DB_FOLDER)) {
    qDebug() << "mkpath() failed:" << ACT_TOPOLOGY_DB_FOLDER;
  */
  if (!QDir().mkpath(topologyDbFolder)) {
    qDebug() << "mkpath() failed:" << topologyDbFolder;
    // kene-
    return std::make_shared<ActStatusInternalError>("Database");
  }

  // kene+
  /*
  if (!QDir().mkpath(ACT_TOPOLOGY_ICON_DB_FOLDER)) {
    qDebug() << "mkpath() failed:" << ACT_TOPOLOGY_ICON_DB_FOLDER;
  */
  QString topologyIconDbFolder = act::database::GetTopologyIconDbFolder();
  if (!QDir().mkpath(topologyIconDbFolder)) {
    qDebug() << "mkpath() failed:" << topologyIconDbFolder;
    // kene-
    return std::make_shared<ActStatusInternalError>("Database");
  }

  return act_status;
}

ACT_STATUS RetrieveData(QSet<ActTopology> &topology_set, qint64 &last_assigned_topology_id) {
  ACT_STATUS_INIT();

  topology_set.clear();

  // Retrieve the data from database
  // kene+
  /*
  QDir dir(ACT_TOPOLOGY_DB_FOLDER);
  */
  QDir dir(act::database::GetTopologyDbFolder());
  // kene-
  QFileInfoList list = dir.entryInfoList(QDir::Files);
  if (list.size() != 0) {
    for (int i = 0; i < list.size(); i++) {
      QString path = list.at(i).absoluteFilePath();
      ActTopology topology;
      act_status = act::database::ReadFromDB(topology, path);
      if (!IsActStatusSuccess(act_status)) {
        qCritical() << "ReadFromDB() failed: topology config database";
        return act_status;
      }

      // [bugfix:2514] AutoScan can not identify device
      topology.DecryptPassword();

      // Assign data to output argument
      topology_set.insert(topology);

      // Update the id to improve the first create performance
      last_assigned_topology_id = topology.GetId();
    }
  }

  return act_status;
}

ACT_STATUS WriteData(const ActTopology &topology) {
  // [bugfix:2514] AutoScan can not identify device
  ActTopology copy_topology = topology;
  copy_topology.EncryptPassword();

  // [feat: 2795] Organize file names in DB
  QString file_name = QString::number(topology.GetId());
  file_name.append("_");
  file_name.append(topology.GetTopologyName());

  // kene+
  /*
  return act::database::WriteToDBFolder<ActTopology>(ACT_TOPOLOGY_DB_FOLDER, file_name, copy_topology,
  */
  return act::database::WriteToDBFolder<ActTopology>(act::database::GetTopologyDbFolder(), file_name, copy_topology,
                                                     // kene-
                                                     topology.key_order_);
}

ACT_STATUS DeleteTopologyFile(const qint64 &id, QString topology_name) {
  // kene+
  /*
  QString icon_name(ACT_TOPOLOGY_ICON_DB_FOLDER);
  */
  QString icon_name(act::database::GetTopologyIconDbFolder());
  // kene-
  icon_name.append("/");
  icon_name.append(QString::number(id));
  icon_name.append(".png");

  // Delete topology icon file
  if (!QFile::remove(icon_name)) {
    qCritical() << "Remove" << icon_name << "failed";
    return std::make_shared<ActStatusInternalError>("Database");
  }

  // Delete topology.json file
  QString file_name = QString::number(id);
  file_name.append("_");
  file_name.append(topology_name);

  // kene+
  /*
  return act::database::DeleteFromFolder(ACT_TOPOLOGY_DB_FOLDER, file_name);
  */
  return act::database::DeleteFromFolder(act::database::GetTopologyDbFolder(), file_name);
  // kene-
}

ACT_STATUS SaveTopologyIcon(const qint64 &id, const QString &url_base64_data) {
  ACT_STATUS_INIT();

  // Upload icon to configuration folder
  // kene+
  /*
  QString file_name(ACT_TOPOLOGY_ICON_DB_FOLDER);
  */
  QString file_name(act::database::GetTopologyIconDbFolder());
  // kene-
  file_name.append("/");
  file_name.append(QString::number(id));
  file_name.append(".PNG");

  // You need to remove the data:image/png;base64, prefix from the start of the string first.
  // example:
  // "data:image/png;base64,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";
  // qDebug() << "url_base64_data:" << url_base64_data.toStdString().c_str();
  QStringList list = url_base64_data.split(",");
  QByteArray base64Data = list[1].toStdString().c_str();
  QImage image;
  image.loadFromData(QByteArray::fromBase64(base64Data), "PNG");
  bool success = image.save(file_name, "PNG");
  if (!success) {
    QString bad_request("Save topology icon file failed");
    qCritical() << bad_request.toStdString().c_str();
    return std::make_shared<ActBadRequest>(bad_request);
  }

  return act_status;
}

}  // namespace topology
}  // namespace database
}  // namespace act
