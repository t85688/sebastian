#pragma once

#include <ctime>
#include <mutex>
#include <unordered_map>

#include "oatpp-websocket/AsyncConnectionHandler.hpp"
#include "oatpp-websocket/AsyncWebSocket.hpp"
#include "oatpp/core/async/Executor.hpp"
#include "oatpp/core/async/Lock.hpp"
#include "oatpp/core/macro/component.hpp"

/**
 * WebSocket listener listens on incoming WebSocket events.
 */
class WSListener : public oatpp::websocket::AsyncWebSocket::Listener {
 private:
  /**
   * Buffer for messages. Needed for multi-frame messages.
   */
  oatpp::data::stream::BufferOutputStream m_messageBuffer;

  /**
   * Lock for synchronization of writes to the web socket.
   */
  oatpp::async::Lock m_writeLock;

  std::shared_ptr<AsyncWebSocket> m_socket;

  qint64 m_id;
  qint64 m_project_id;

  oatpp::async::Executor m_asyncExecutor;

 public:
  // WSListener() {}
  WSListener(const std::shared_ptr<AsyncWebSocket> &socket, const qint64 &id, const qint64 &project_id)
      : m_socket(socket), m_id(id), m_project_id(project_id) {}

  /**
   * Called on "ping" frame.
   */
  CoroutineStarter onPing(const std::shared_ptr<AsyncWebSocket> &socket, const oatpp::String &message) override;

  /**
   * Called on "pong" frame
   */
  CoroutineStarter onPong(const std::shared_ptr<AsyncWebSocket> &socket, const oatpp::String &message) override;

  /**
   * Called on "close" frame
   */
  CoroutineStarter onClose(const std::shared_ptr<AsyncWebSocket> &socket, v_uint16 code,
                           const oatpp::String &message) override;

  /**
   * Called on each message frame. After the last message will be called once-again with size == 0 to designate end of
   * the message.
   */
  CoroutineStarter readMessage(const std::shared_ptr<AsyncWebSocket> &socket, v_uint8 opcode, p_char8 data,
                               oatpp::v_io_size size) override;

  /**
   * Send message to peer (client).
   */
  void sendMessage(const oatpp::String &message);

  /**
   * @brief Get the id object
   *
   * @param id
   */
  void getId(qint64 &id);

  /**
   * @brief Get the project_id object
   *
   * @param project_id
   */
  void getProjectId(qint64 &project_id);
};

/**
 * Listener on new WebSocket connections.
 */
class WSInstanceListener : public oatpp::websocket::AsyncConnectionHandler::SocketInstanceListener {
 public:
  /**
   * Counter for connected clients.
   */
  static std::atomic<v_int32> SOCKETS;

  /**
   *  Called when socket is created
   */
  void onAfterCreate_NonBlocking(const std::shared_ptr<AsyncWebSocket> &socket,
                                 const std::shared_ptr<const ParameterMap> &params) override;

  /**
   *  Called before socket instance is destroyed.
   */
  void onBeforeDestroy_NonBlocking(const std::shared_ptr<AsyncWebSocket> &socket) override;
};
