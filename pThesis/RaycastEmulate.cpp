#include "RaycastEmulate.h"



void CPURaycaster::Init(const Int2 _numThreads, const Int2 _resolution, SVO_Loader* _pSvoLoader)
{
	m_numThreads = _numThreads;
	m_resolution = _resolution;
	m_pSvoLoader = _pSvoLoader;
}

void CPURaycaster::Emulate(const CBDCamera& _camera)
{
	camera = _camera;
	int xmax, ymax;
	xmax = ymax = 0;

	for (int i = 0; i < m_numThreads.x * 32 + 1; ++i)
	{
		for (int j = 0; j < m_numThreads.y * 16 + 1; ++j)
		{
			Int2 threadId;
			threadId.x = i;
			threadId.y = j;

			EmulateThread(threadId);
		}
	}

	printf("%i, %i\n", xmax, ymax);
}

void CPURaycaster::EmulateThread(const Int2 _threadId)
{
	struct Ray					// 48 bytes
	{
		FVEC3 origin;			// 12 bytes
		float origin_sz;		// 4
		FVEC3 direction;		// 12 bytes	
		float direction_sz;		// 4
	};

	Ray ray;

	float y = float(2.f * _threadId.y + 1.f - m_resolution.y) * (1.f / (float)m_resolution.y) + 1;
	float x = float(2.f * _threadId.x + 1.f - m_resolution.x) * (1.f / (float)m_resolution.x) + 1;
	float z = 1.f;
	XMFLOAT4 aux;
	XMStoreFloat4(&aux, XMVector4Transform(XMLoadFloat4(&XMFLOAT4(1, 1, 1, 1.f)), camera.mViewInverse));

	ray.origin = FVEC3(aux.x, aux.y, aux.z) / aux.w;

	XMFLOAT3 pp;
	XMStoreFloat3(&pp, XMVector4Transform(XMLoadFloat4(&XMFLOAT4(x, y, z, 1)), camera.mViewInverse));
	ray.direction = VECTOR::Normalize(FVEC3(pp.x, pp.y, pp.z) - ray.origin);

	// TRAVERSE
	float tmin, tmax;
	tmin = 42;
}