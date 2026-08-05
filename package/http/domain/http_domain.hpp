//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2023
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include "http_config.hpp"
#include <http/ihttp.hpp>
#include <wfc/domain_object.hpp>
#include <memory>

namespace wfc{

class http_domain
  : public wfc::domain_object<ihttp, http_config>
  , public std::enable_shared_from_this<http_domain>
{
  class connection;
  typedef wfc::domain_object<ihttp, http_config> super;
public:
  virtual ~http_domain();
  http_domain();

  virtual void configure() override;
  virtual void initialize() override;
  virtual void reg_io(io_id_t io_id, std::weak_ptr<iinterface> itf) override;
  virtual void unreg_io(io_id_t io_id) override;
  virtual void perform_io(data_ptr d, io_id_t io_id, output_handler_t handler) override;

private:
  typedef std::mutex mutex_type;
  typedef std::shared_ptr<connection> connection_ptr;
  typedef std::weak_ptr<connection> connection_wptr;
  typedef std::unordered_map<io_id_t, connection_ptr> connection_map_t;

private:
  connection_ptr find_connection(io_id_t id) const;
  void post_next_(connection_wptr wconn, output_handler_t handler);
  void perform_next_(connection_wptr wconn, output_handler_t handler);


  http_config _opt;
  mutable mutex_type _mutex;
  connection_map_t _connection_map;
  std::weak_ptr<iinterface> _target;
};

}
