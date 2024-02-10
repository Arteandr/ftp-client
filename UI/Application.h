#pragma once

#include <UI/Layer.h>

#include <vector>
#include <memory>

namespace UI {
	class Application {
	public:
		template<typename T>
		void PushLayer() {
			static_assert(std::is_base_of<Layer, T>::value, "pushed type is not subclass of layer");
			m_LayerStack.emplace_back(std::make_shared<T>())->OnAttach();
		};
	
	public:
		std::vector<std::shared_ptr<Layer>> m_LayerStack;
		bool m_Running;
	};
}
