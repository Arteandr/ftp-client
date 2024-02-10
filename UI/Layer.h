#pragma once

namespace UI {

	class Layer {
	public: 
		virtual ~Layer() = default;

		virtual void OnAttach() {}
		virtual void OnDetach() {}

		virtual void OnUpdate() {}
		virtual void OnUIRender() {}
	};

}
