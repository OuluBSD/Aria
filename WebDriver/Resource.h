#ifndef _WebDriver_Resource_h_
#define _WebDriver_Resource_h_

#include <Core/Core.h>
#include "ResponseStatusCode.h"

NAMESPACE_UPP

namespace detail {

class Resource : public SharedObjectBase { // noncopyable
public:
	enum Ownership { IS_OWNER, IS_OBSERVER };

	Resource(
		const String& url,
		Ownership mode = IS_OBSERVER
		)
		: url_(url)
		, ownership_(mode)
	{}

	Resource(
		const Shared<Resource>& parent,
		const String& name,
		Ownership mode = IS_OBSERVER
		)
		: parent_(parent)
		, url_(ConcatUrl(parent->url_, name))
		, ownership_(mode)
	{}

	virtual ~Resource() {
		try {
			if (ownership_ == IS_OWNER)
				DeleteResource();
		} catch (const std::exception&) {}
	}

	const String& GetUrl() const {
		return url_;
	}

	Value Get(const String& command = String()) const {
		return Download(command, HttpRequest::METHOD_GET, "GET");
	}

	template<typename T>
	T GetValue(const String& command) const {
		try {
			return FromJson<T>(Get(command));
		} catch (const std::exception& ex) {
			throw std::runtime_error(String(ex.what()) + " - Context: command: " + command);
		}
	}

	String GetString(const String& command) const {
		return GetValue<String>(command);
	}

	bool GetBool(const String& command) const {
		return GetValue<bool>(command);
	}

	Value Delete(const String& command = String()) const {
		return Download(command, HttpRequest::METHOD_DELETE, "DELETE");
	}

	Value Post(
		const String& command = String(),
		const Value& upload_data = Value()
		) const {
		return Upload(command, upload_data, HttpRequest::METHOD_POST, "POST");
	}

	template<typename T>
	void Post(
		const String& command,
		const String& arg_name,
		const T& arg_value
		) const {
		ValueMap obj;
		obj.Add(arg_name, ToJson(arg_value));
		Post(command, Value(obj));
	}

	template<typename T>
	void PostValue(const String& command, const T& value) const {
		try {
			Post(command, ToJson(value));
		} catch (const std::exception& ex) {
			throw std::runtime_error(String(ex.what()) + " - Context: command: " + command);
		}
	}

protected:
	virtual Value TransformResponse(Value& response) const {
		return response["value"];
	}

	virtual void DeleteResource() {
		Delete();
	}

private:
	Value Download(
		const String& command,
		int method,
		const char* request_type
		) const {
		try {
			String url = ConcatUrl(url_, command);
			RLOG("WebDriver: " << request_type << " " << url);
			HttpRequest req(url);
			req.Method(method);
			req.Timeout(RequestTimeoutMs(command));
			String body = req.Execute();
			return ProcessResponse(req.GetStatusCode(), body);
		} catch (const std::exception& ex) {
			throw std::runtime_error(String(ex.what()) + " - Context: "
				+ "request: " + request_type
				+ ", command: " + command
				+ ", resource: " + url_
				);
		}
	}

	static String ToUploadData(const Value& upload_data)
	{
		return IsNull(upload_data) ? String() : AsJSON(upload_data);
	}

	Value Upload(
		const String& command,
		const Value& upload_data,
		int method,
		const char* request_type
		) const {
		try {
			String url = ConcatUrl(url_, command);
			RLOG("WebDriver: " << request_type << " " << url);
			HttpRequest req(url);
			req.Method(method);
			req.Timeout(RequestTimeoutMs(command));
			if (!IsNull(upload_data)) {
				req.PostData(ToUploadData(upload_data));
				req.ContentType("application/json; charset=UTF-8");
			}
			String body = req.Execute();
			return ProcessResponse(req.GetStatusCode(), body);
		} catch (const std::exception& ex) {
			throw std::runtime_error(String(ex.what()) + " - Context: "
				+ "request: " + request_type
				+ ", command: " + command
				+ ", resource: " + url_
				+ ", data: " + ToUploadData(upload_data)
				);
		}
	}

	Value ProcessResponse(
		int http_code,
		const String& body
		) const {
		try {
			if (http_code / 100 != 2 && http_code / 100 != 4 && http_code != 500 && http_code != 501) {
				throw std::runtime_error("Unsupported HTTP code: " + AsString(http_code));
			}

			if (body.IsEmpty()) {
				if (http_code >= 400) {
					throw std::runtime_error("Server error: HTTP code " + AsString(http_code) + " (Empty body)");
				}
				return Value();
			}

			Value response = ParseJSON(body);
			if (response.IsError()) {
				throw std::runtime_error("JSON parser error: " + body);
			}

			if (!response.Is<ValueMap>()) throw std::runtime_error("Server response is not an object");
			ValueMap res_map = response;
			
			// W3C status code is optional, legacy is required.
			int status_code = 0;
			if (res_map.Find("status") >= 0) {
				Value s = res_map["status"];
				if (s.Is<double>() || s.Is<int>())
					status_code = (int)s;
			}
			
			const auto status = static_cast<response_status_code::Value>(status_code);
			
			Value value;
			if (res_map.Find("value") >= 0) {
				value = res_map["value"];
			}

			if (http_code >= 400 || status != response_status_code::kSuccess) {
				String message = "Unknown error";
				if (value.Is<ValueMap>()) {
					ValueMap v_map = value;
					if (v_map.Find("message") >= 0) message = AsString(v_map["message"]);
					else if (v_map.Find("error") >= 0) message = AsString(v_map["error"]);
				} else if (!value.IsVoid()) {
					message = AsString(value);
				}
				
				throw std::runtime_error(String("Server failed to execute command (")
					+ "message: " + message
					+ ", status: " + response_status_code::ToString(status)
					+ ", status_code: " + AsString((int)status)
					+ ")"
				);
			}

			Value res;
			res = response;
			return TransformResponse(res);
		} catch (const std::exception& ex) {
			throw std::runtime_error(String(ex.what()) + " - Context: "
				+ "HTTP code: " + AsString(http_code)
				+ ", body: " + body
				);
		}
	}

	//A wedged geckodriver (browser blocked on a modal, session never becoming ready) must not
	//hang the CLI forever. Session creation gets a short budget, every other command a generous
	//one that still bounds a genuine deadlock.
	static int RequestTimeoutMs(const String& command) {
		return command == "session" ? 60000 : 300000;
	}

	static
	String ConcatUrl(const String& a, const String& b, const char delim = '/') {
		auto result = a.IsEmpty() ? b : a;
		if (!a.IsEmpty() && !b.IsEmpty()) {
			if (result[result.GetLength()-1] != delim)
				result.Cat(delim);
			result.Cat(b[0] == delim ? b.Mid(1) : b);
		}
		return result;
	}

private:
	const Shared<Resource> parent_;
	const String url_;
	const Ownership ownership_;
};

class Root_resource : public Resource { // noncopyable
public:
	Root_resource(
		const String& url
		)
		: Resource(url, IS_OBSERVER)
	{}

private:
	virtual Value TransformResponse(Value& response) const {
		Value res;
		res = response;
		return res;
	}
};

inline
Shared<Resource> MakeSubResource(
	const Shared<Resource>& parent,
	const String& subpath,
	Resource::Ownership mode = Resource::IS_OBSERVER
	) {
	return Shared<Resource>(new Resource(parent, subpath, mode));
}

inline
Shared<Resource> MakeSubResource(
	const Shared<Resource>& parent,
	const String& subpath1,
	const String& subpath2,
	Resource::Ownership mode = Resource::IS_OBSERVER
	) {
	return Shared<Resource>(new Resource(parent, subpath1 + "/" + subpath2, mode));
}

} // namespace detail

END_UPP_NAMESPACE

#endif