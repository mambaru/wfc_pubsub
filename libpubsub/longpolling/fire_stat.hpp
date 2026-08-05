#pragma once

#include <longpolling/hub_queue_stat.hpp>
#include <cstddef>
#include <ostream>

namespace wfc{ namespace pubsub{

struct fire_stat
{
  // Все отправленные сообщения
  size_t sended_messages = 0;
  // Сообщения ожидающие подверждения о доставке
  size_t wait_messages = 0;
  // Сообщения хранимые в агентах
  size_t stored_messages = 0;
  // Удалено устаревших сообщений
  size_t remove_death = 0;
  //  все каналы со всех агентов включая дубликаты
  size_t active_channels = 0;
  // Активные агенты
  size_t active_agents = 0;
  // Карта каналов для агентов
  size_t active_agents_uc = 0;

  // Удаленные агенты
  size_t deleted_agents = 0;
  // Отписки для удааленных агентов
  size_t dead_describe = 0;

  // Сообщения хранимые в агентах
  size_t hub_stored_messages = 0;
  //  все каналы со всех агентов включая дубликаты
  size_t hub_active_channels = 0;
  // Удалено устаревших сообщений из общего хаба
  size_t hub_remove_death = 0;

  hub_queue_stat hub_queue;

  fire_stat& operator += (const fire_stat& other)
  {
    // Все отправленные сообщения
    this->sended_messages += other.sended_messages;
    // Сообщения ожидающие подверждения о доставке
    this->wait_messages = other.wait_messages;
    // Сообщения хранимые в агентах
    this->stored_messages = other.stored_messages;
    // Удалено устаревших сообщений
    this->remove_death += other.remove_death;
    //  все каналы со всех агентов включая дубликаты
    this->active_channels = other.active_channels;
    // Активные агенты
    this->active_agents = other.active_agents;
    this->active_agents_uc = other.active_agents_uc;
    // Удаленные агенты
    this->deleted_agents += other.deleted_agents;
    this->dead_describe += other.dead_describe;
    // Сообщения хранимые в агентах
    this->hub_stored_messages = other.hub_stored_messages;
    //  все каналы со всех агентов включая дубликаты
    this->hub_active_channels = other.hub_active_channels;
    // Удалено устаревших сообщений из общего хаба
    this->hub_remove_death += other.hub_remove_death;
    this->hub_queue += other.hub_queue;
    return *this;
  }
};

inline std::ostream& operator<< ( std::ostream& os, const fire_stat& fs)
{
  os << "\tMessages sended: " << fs.sended_messages << ", wait: " << fs.wait_messages
     << ", stored: " << fs.stored_messages << ", removed: " << fs.remove_death << std::endl;
  os << "\tChannels: " << fs.active_channels << ", agents: " << fs.active_agents << ", deleted:" << fs.deleted_agents << std::endl;
  os << "\tHub channels: " << fs.hub_active_channels << ", stored messages: " << fs.hub_stored_messages << ", removed:" << fs.hub_remove_death << std::endl;

  return os;
}

}}
