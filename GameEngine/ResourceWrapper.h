#pragma once
#include <d3dx12.h>
#include <iostream>

class ResourceWrapper
{
public:
	ResourceWrapper(D3D12_RESOURCE_STATES state)
	{
		m_currentState = state;
	}

	void TransitionState(ID3D12GraphicsCommandList* cmdList, D3D12_RESOURCE_STATES nextState)
	{
		if (nextState == m_currentState)
			return;

		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			m_resource.Get(),
			m_currentState,
			nextState
		);

		cmdList->ResourceBarrier(1, &barrier);

		m_currentState = nextState;
	}

	void SetCurrentState(D3D12_RESOURCE_STATES state)
	{
		m_currentState = state;
	}

	D3D12_RESOURCE_STATES GetCurrentState() const
	{
		return m_currentState;
	}

	ID3D12Resource* GetResource() const
	{
		return m_resource.Get();
	}

	ID3D12Resource** GetAddressOf()
	{
		return m_resource.GetAddressOf();
	}

	ID3D12Resource** ReleaseAndGetAddressOf()
	{
		return m_resource.ReleaseAndGetAddressOf();
	}

	Microsoft::WRL::ComPtr<ID3D12Resource> GetComPtrResource() const
	{
		return m_resource;
	}

private:
	D3D12_RESOURCE_STATES m_currentState;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_resource;
};
