#include "Game/ChessObject.hpp"
#include "Engine/Math/Mat44.hpp"

ChessObject::ChessObject(ChessMatch* match)
	: m_match(match)
{

}

Mat44 ChessObject::GetModelToWorldTransform() const
{
	Mat44 result = m_orientation.GetAsMatrix_IFwd_JLeft_KUp();
	result.AppendScaleNonUniform3D(m_scale);
	result.SetTranslation3D(m_position);
	return result;
}
