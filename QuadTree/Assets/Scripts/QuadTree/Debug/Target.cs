using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Events;
using UnityEngine.EventSystems;


public class Target : MonoBehaviour, IPointerClickHandler
{
    public void OnPointerClick(PointerEventData eventData)
    {
        Debug.Log("OnPointerClick");
    }

    private void OnMouseDown()
    {
        Debug.Log("OnMouseDown");
    }

    private void OnMouseDrag()
    {
        //Debug.Log(Input.mousePosition);
    }

    private void OnMouseUp()
    {
        Debug.Log("OnMouseUp");
    }
}