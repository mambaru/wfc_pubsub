
#include "http_domain.hpp"
#include "../logger.hpp"
#include <boost/beast.hpp>
#include <boost/url.hpp>

#include <iostream>
namespace wfc{

class http_domain::connection
  : public iinterface
{
public:
  typedef iow::io::data_type data_type;
  typedef data_type::value_type char_type;
  typedef boost::beast::flat_buffer buffer_type;
  typedef boost::beast::http::vector_body<data_type::value_type> body_type;
  typedef boost::beast::http::request_parser<body_type> parser_type;
  typedef std::shared_ptr<parser_type> parser_ptr;
  typedef parser_type::value_type message_type;
  typedef boost::system::error_code error_code;
  typedef std::mutex mutex_type;

  connection(std::weak_ptr<iinterface> wconn)
    : _wconn(wconn)
  {

  }
  void put(const data_type& d)
  {
    std::lock_guard<mutex_type> lk(_mutex);

    boost::asio::mutable_buffer current_buffer = _buffer.prepare(d.size());
    std::copy(d.begin(), d.end(), static_cast<char_type*>(current_buffer.data()) );
    _buffer.commit(d.size());

    for (;;)
    {
      error_code ec;
      if ( _parser == nullptr )
      {
        const char* beg = static_cast<const char*>(_buffer.cdata().data());
        const char* end = beg + _buffer.cdata().size();
        const char* off = wjson::parser::parse_space(beg, end, nullptr);
        _buffer.consume(static_cast<size_t>(std::distance(beg, off)) );
        if ( off == end )
          return;
        _parser = std::make_shared<parser_type>();
      }

      size_t parsed_chars = _parser->put(_buffer.data(), ec);
      _buffer.consume(parsed_chars);
      if ( ec || parsed_chars==0)
        return;

      if ( _parser->is_done() )
      {
        _messages.push_back( _parser->release() );
        // Для каждого сообщения новый парсер
        _parser = nullptr;
      }
    }
  }

  std::unique_ptr<message_type> fetch()
  {
    std::unique_ptr<message_type> msg;

    std::lock_guard<mutex_type> lk(_mutex);

    if ( _bussy )
      return msg;

    if ( _messages.empty() )
      return msg;

    _bussy = true;
    msg = std::make_unique<message_type>( std::move(_messages.front()) );
    _messages.pop_front();
    return msg;
  }

  virtual void perform_io(data_ptr d, io_id_t io_id, output_handler_t /*handler*/) override
  {
    // обработка встрецных вызовов
    this->send_response( *d, [this, io_id](data_ptr dd){
      if ( auto pconn = this->_wconn.lock() )
      {
        this->perform_io( std::move(dd), io_id, nullptr );
      }
    });
  }

  using response_body_t  = boost::beast::http::response<boost::beast::http::buffer_body>;
  using response_empty_t = boost::beast::http::response<boost::beast::http::empty_body>;

  template<typename Body>
  void send_response_(const data_type& d, const output_handler_t& handler)
  {
    boost::beast::http::response<Body> res;
    res.version(11);
    res.result(boost::beast::http::status::ok);
    res.set(boost::beast::http::field::content_length, std::to_string(d.size()) );
    res.set(boost::beast::http::field::access_control_allow_origin, "*");
    res.set(boost::beast::http::field::access_control_allow_methods, "GET,POST");
    res.set(boost::beast::http::field::access_control_allow_headers, "X-Accept-Charset, X-Accept, Content-Type");
    res.set(boost::beast::http::field::content_type, "application/json; charset=utf-8");
    //res.set(boost::beast::http::field::server, "Comet");
    this->make_body_(&res, d);
    boost::beast::http::response_serializer<Body, boost::beast::http::fields> sr{res};
    while( !sr.is_done() )
    {
      boost::system::error_code ec;
      size_t sended = 0;
      sr.next(ec, [/*&sended,*/ handler](boost::system::error_code& , const auto& bufs){
        std::deque<char>  dd;
        for ( const auto& buf: bufs)
        {
          std::copy(
            static_cast<const char*>(buf.data()),
            static_cast<const char*>(buf.data()) + buf.size(),
            std::back_inserter(dd));
          /*sended += buf.size();
          const char* ch = static_cast<const char*>(buf.data());
          handler(std::make_unique<data_type>(ch, ch + buf.size()));*/
        }
        handler(std::make_unique<data_type>(dd.begin(),dd.end()));
      });
      // std::cout << "-1.1-" << ec.message() << std::endl;

      if ( sended == 0 )
        break;

      sr.consume(sended);
    }
    _bussy = false;
  }

  void make_body_(response_body_t* res, const data_type& d)
  {
    res->body().size = d.size();
    res->body().data = const_cast<void*>( static_cast<const void*>(d.data())); //buf;
  }

  void make_body_(response_empty_t* , const data_type& )
  {

  }


  void send_response(const data_type& d, const output_handler_t& handler)
  {
    if ( d.empty() )
      this->send_response_<boost::beast::http::empty_body>(d, handler);
    else
      this->send_response_<boost::beast::http::buffer_body>(d, handler);
  }

  void send_response(const output_handler_t& handler)
  {
    this->send_response_<boost::beast::http::empty_body>(data_type(), handler);
  }

private:
  mutex_type _mutex;
  bool _bussy = false;
  buffer_type _buffer;
  std::deque<message_type> _messages;
  parser_ptr _parser;
  std::weak_ptr<iinterface> _wconn;
};

http_domain::~http_domain()
{
}

http_domain::http_domain()
{
}

void http_domain::configure()
{
  _opt = this->options();
}

void http_domain::initialize()
{
  _target = this->get_target<iinterface>( _opt.target);
}



void http_domain::reg_io(io_id_t io_id, std::weak_ptr<iinterface> itf)
{
  std::lock_guard<mutex_type> lk(_mutex);
  auto pconn = std::make_shared<connection>(itf);
  _connection_map[io_id] = pconn;
  if ( auto p = _target.lock() )
    p->reg_io(io_id, pconn);
}

void http_domain::unreg_io(io_id_t io_id)
{
  std::lock_guard<mutex_type> lk(_mutex);
  _connection_map.erase(io_id);
  if ( auto p = _target.lock() )
    p->unreg_io(io_id);
}

void http_domain::perform_io(data_ptr d, io_id_t io_id, output_handler_t handler)
{
  if ( connection_ptr conn = this->find_connection(io_id) )
  {
    conn->put(*d);
    this->post_next_(conn, handler);
  }
}

void http_domain::post_next_(connection_wptr wconn, output_handler_t handler)
{
  this->get_workflow()->safe_post(std::bind(&http_domain::perform_next_, this, wconn, handler));
}

namespace {
    struct jsonrpc_id{
      std::string method;
      int id = -1;
    };

    struct jsonrpc_id_json
    {
      JSON_NAME(method)
      JSON_NAME(id)
      typedef wjson::object<
        jsonrpc_id,
        wjson::member_list<
          wjson::member<n_method, jsonrpc_id, std::string, &jsonrpc_id::method>,
          wjson::member<n_id, jsonrpc_id, int, &jsonrpc_id::id>
        >
      > meta;
      typedef meta::serializer serializer;
    };
}

void http_domain::perform_next_(connection_wptr wconn, output_handler_t handler)
try
{
  if ( auto pconn = wconn.lock() )
  {
    if (auto msg = pconn->fetch() )
    {
      boost::urls::url_view uv(msg->target());
      data_type result_data;

      if ( uv.path() == _opt.jsonrpc_path )
      {
        auto pcontent = std::make_unique<data_type>(msg->body());
        // TODO сделать разбор GET параметров и json формат
        if ( auto ptarget = _target.lock() )
        {
          jsonrpc_id jid;
          jsonrpc_id_json::serializer()(jid, pcontent->begin(), pcontent->end(), nullptr);

          // Костыль для старого комета
          bool counter_call = jid.id==-1 && (jid.method == "create" || jid.method == "subscribe");
          if ( counter_call )
          {
            ptarget->perform_io(std::move(pcontent), this->get_id(), [wconn, handler](data_ptr d)
            {
              // встречный вызов
              if ( auto pc = wconn.lock() )
              {
                if ( d!=nullptr )
                  pc->send_response( *d, handler);
                else
                  pc->send_response(handler);
              }
            });
          }
          else if ( jid.id == -1)
          {
            // Уведомление
            ptarget->perform_io(std::move(pcontent), this->get_id(), [](data_ptr d){
              HTTP_LOG_WARNING("Недопустимый вызов обработчика для jsonrpc :" << d)
            });
            pconn->send_response(handler);
            this->post_next_(pconn, handler);
          }
          else
          {
            // Запрос
            ptarget->perform_io(std::move(pcontent), this->get_id(), [this, handler, wconn](data_ptr d)
            {
              if ( auto pc = wconn.lock() )
              {
                pc->send_response(*d, handler);
                this->post_next_(wconn, handler);
              }
            });
          }
        }
      } // if ( uv.path() == _opt.jsonrpc_path )
    }
  }
}
catch (const boost::system::system_error& e)
{
  HTTP_LOG_ERROR("Boost System Exception: " << e.what() );
  if ( auto pconn = wconn.lock() )
  {
    pconn->send_response(handler);
    this->post_next_(pconn, handler);
  }
}
catch (const std::exception& e)
{
  HTTP_LOG_ERROR("Runtime Exception: " << e.what() );
  if ( auto pconn = wconn.lock() )
  {
    pconn->send_response(handler);
    this->post_next_(pconn, handler);
  }
}

http_domain::connection_ptr http_domain::find_connection(io_id_t id) const
{
  std::lock_guard<mutex_type> lk(_mutex);
  auto itr = _connection_map.find(id);
  if ( itr != _connection_map.end() )
    return itr->second;
  return nullptr;
}


}

